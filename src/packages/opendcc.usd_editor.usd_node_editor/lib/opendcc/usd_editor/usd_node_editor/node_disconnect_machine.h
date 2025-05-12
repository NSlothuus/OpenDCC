/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include <QTimer>
#include <unordered_map>
#include <memory>

OPENDCC_NAMESPACE_OPEN

/// Provides a Finite State Machine (FSM) to detect shaking of the mouse cursor
/// and perform node disconnection based on specific cursor movements and triggers.
///
/// The FSM consists of distinct states representing various stages of the shake detected
/// and disconnecting process, with transitions triggered by specific cursor actions.
///
/// \section Disconnect Finite State Machine
/// The DisconnectFSM is a Finite State Machine designed to handle the process of disconnecting nodes
/// after detecting shaking of the mouse cursor.
/// It consists of states and triggers that guide the transition between states during the shaking process.
///
/// The FSM has functionalities like starting, stopping, updating,
/// and restarting to manage the disconnecting process.
/// Each state has methods to update its behavior based on triggers
/// and to perform specific actions during its phase of the disconnecting process.
/// The FSM orchestrates these state updates and actions by transitioning
/// between states according to the detected cursor movements and triggers.
///
/// \subsection States
/// The FSM includes states each representing different stages of the shake detection
/// and disconnecting process based on the mouse cursor's movements.
///
/// enum State          | class State
/// ------------------- | ------------
/// Nulled              | NullState
/// Pressed             | PressedState
/// StartArea           | StartAreaState
/// AfterRightBorder    | AfterBorderLineState
/// AfterLeftBorder     | AfterBorderLineState
/// AfterRightmostPoint | AfterChangeDirectionPointState
/// AfterLeftmostPoint  | AfterChangeDirectionPointState
/// Disconnected        | DisconnectedState
///
/// \subsection Disconnect FSM Transition Table
/// State transition table from DisconnectState based on triggers from DisconnectTrigger.
///
/// Current state       | Trigger                           | Next State
/// ------------------- | --------------------------------- | ----------
/// Nulled              | Press                             | Pressed
/// Pressed             | Release                           | Nulled
/// Pressed             | ToStart                           | StartArea
/// StartArea           | Release                           | Nulled
/// StartArea           | LeftStartArea                     | AfterRightBorder
/// StartArea           | LeftStartArea                     | AfterLeftBorder
/// AfterRightBorder    | Release                           | Nulled
/// AfterRightBorder    | DirectionChangeAtRightmostPoint   | AfterRightmostPoint
/// AfterLeftBorder     | Release                           | Nulled
/// AfterLeftBorder     | DirectionChangeAtLeftmostPoint    | AfterLeftmostPoint
/// AfterRightmostPoint | Release                           | Nulled
/// AfterRightmostPoint | RightToLeft                       | AfterLeftBorder
/// AfterRightmostPoint | Disconnect                        | Disconnected
/// AfterLeftmostPoint  | Release                           | Nulled
/// AfterLeftmostPoint  | LeftToRight                       | AfterRightBorder
/// AfterLeftmostPoint  | Disconnect                        | Disconnected
/// Disconnected        | Release                           | Nulled
///
/// At the "StartArea" state, when "LeftStartArea" is triggered, the state checks the movement direction
/// and transitions to "AfterRightBorder" if it is to right, and to "AfterLeftBorder" otherwise.
///
/// \subsection General scenarios of the FSM operation
/// The main scenarios of transition from the Null State to the Disconnected State:
/// - The FSM starts in the "Nulled" State by default.
/// - If a mouse is pressed on a node, a "Press" trigger is activated,
///   and the FSM transitions to the "Pressed" State.
/// - If the mouse starts moving in the "Pressed" State and receives a "ToStart" trigger,
///   the FSM progresses to the "StartArea" State.
/// - In the "StartArea" State, if a node is moved to the right(left) beyond a certain distance,
///   the FSM transitions to the "AfterRightBorder"("AfterLeftBorder") State.
/// - While moving the mouse further in the "AfterRightBorder"("AfterLeftBorder") State
///   and changing direction when reaching the rightmost(leftmost) point, it leads to a transition
///   to the "AfterRightmostPoint"("AfterLeftmostPoint") State.
/// - In the "AfterRightmostPoint"("AfterLeftmostPoint") State,
///   if a node is moved to the left(right) beyond a certain distance,
///   a "RightToLeft"("LeftToRight")trigger is activated,
///   and the FSM transitions to the "AfterLeftBorder"("AfterRightBorder") State.
/// - While moving the mouse further in the "AfterLeftBorder"("AfterRightBorder") State
///   and changing direction when reaching the leftmost(rightmost) point,
///   it leads to a transition to the "AfterLeftmostPoint"("AfterRightmostPoint") State.
/// - In the "AfterLeftmostPoint"("AfterRightmostPoint") State, if a "Disconnect" trigger is received
///   and the node is moved to the right(left) beyond a certain distance,
///   the FSM transitions to the "Disconnected" State, where the node is disconnected.
///
/// If the mouse is released at any time, the "Release" trigger will fire and the FSM state will be reset to "Nulled".
/// If this scenario does not end by a certain time from the start of movement, then the FSM will restart,
/// and the current cursor position will become the starting point.

/**
 * @enum DisconnectState
 * @brief Enum class representing different states of the finite-state machine for disconnecting nodes after shaking.
 */
enum class DisconnectState : int
{
    Nulled = 0, ///< Initial null state
    Pressed, ///< State when user presses on a node
    StartArea, ///< State when cursor is on the start area
    AfterRightBorder, ///< State after cursor moves past the right border
    AfterLeftBorder, ///< State after cursor moves past the left border
    AfterRightmostPoint, ///< State after cursor change direction at the rightmost point
    AfterLeftmostPoint, ///< State after cursor change direction at the leftmost point
    Disconnected ///< State when the node is disconnected
};

/**
 * @enum DisconnectTrigger
 * @brief Enum class representing triggers for the transitions of the finite-state machine for disconnecting nodes after shaking.
 */
enum class DisconnectTrigger : int
{
    Press = 0, ///< Trigger when mouse button is pressed
    ToStart, ///< Trigger when mouse starts moving at start area
    LeftStartArea, ///< Trigger when mouse leaves the start area
    RightToLeft, ///< Trigger when mouse moves from right to left between extreme points
    LeftToRight, ///< Trigger when mouse moves from left to right between extreme points
    DirectionChangeAtRightmostPoint, ///< Trigger when mouse changes direction at rightmost point
    DirectionChangeAtLeftmostPoint, ///< Trigger when mouse changes direction at leftmost point
    Disconnect, ///< Trigger when the node shaking ends and needs to be disconnected
    Release ///< Trigger when mouse button is released
};

/**
 * @struct DisconnectData
 * @brief Structure to hold data related with a finite-state machine for disconnecting nodes after shaking.
 */
struct DisconnectData
{
    /*
     * @brief Clears all data stored in the struct.
     */
    void clear_data();

    /**
     * @brief Sets the current x-coordinate of the cursor.
     * @param x The new x-coordinate.
     */
    void set_current_x(int x);

    /**
     * @brief Sets the mouse press x-coordinate of the cursor.
     * @param x The new start x-coordinate.
     */
    void set_start_x(int x);

    /**
     * @brief Sets the starting direction of the disconnection movement.
     * @param is_right True if the starting direction is to the right, false if it is to the left.
     */
    void set_start_direction(bool is_right);

    /**
     * @brief Sets the value of the current x to the value of the rightmost or leftmost x.
     * @param is_rightmost True if the remembered point is rightmost, false if it is leftmost.
     */
    void remember_change_direction_x(bool is_rightmost);

    /**
     * @brief Gets the starting x-coordinate of the shake movement.
     * @return The starting x-coordinate.
     */
    int get_start_x() const { return m_start_x; }

    /**
     * @brief Gets the current x-coordinate of the cursor.
     * @return The current x-coordinate.
     */
    int get_current_x() const { return m_current_x; }

    /**
     * @brief Gets the previous x-coordinate of the cursor.
     * @return The previous x-coordinate.
     */
    int get_old_x() const { return m_old_x; }

    /**
     * @brief Gets the x-coordinate of the cursor at 2 steps back.
     * @return The previous-previous x-coordinate.
     */
    int get_old_old_x() const { return m_old_x; }

    /**
     * @brief Gets the rightmost x-coordinate reached during shaking.
     * @return The rightmost x-coordinate.
     */
    int get_rightmost_x() const { return m_rightmost_x; }

    /**
     * @brief Gets the leftmost x-coordinate reached during shaking.
     * @return The leftmost x-coordinate.
     */
    int get_leftmost_x() const { return m_leftmost_x; }

    /**
     * @brief Checks if the direction of the shaking has changed.
     * @return True if the direction has changed, false otherwise.
     */
    bool is_direction_changed() const { return (m_old_old_x - m_old_x) * (m_old_x - m_current_x) < 0; }

    /**
     * @brief Checks if the starting shake movement was to the right.
     * @return True if the starting shake movement was to the right, false otherwise.
     */
    bool is_start_move_to_right() const { return m_start_move_to_right; }

    QTimer* timer = nullptr;

private:
    int m_start_x = 0;
    bool m_start_move_to_right = false;

    int m_current_x = 0;
    int m_old_x = 0;
    int m_old_old_x = 0;

    int m_rightmost_x = 0;
    int m_leftmost_x = 0;
};

class UsdPrimNodeItemBase;
class DisconnectFSMState;

/**
 * @class DisconnectFSM
 * @brief Class representing of the Finite State Machine for disconnecting nodes after shaking.
 */
class DisconnectFSM
{
public:
    /**
     * @brief Constructor for DisconnectFSM.
     * @param node Pointer to UsdPrimNodeItemBase object.
     */
    DisconnectFSM(UsdPrimNodeItemBase* node);

    /**
     * @brief Get the current state of the FSM.
     * @return Pointer to the current FSM state.
     */
    DisconnectFSMState* get_current_state() const { return m_current_state; }

    /**
     * @brief Set the state of the FSM to a new state.
     * @param new_state The new state to set.
     */
    void set_state(DisconnectState new_state);

    /**
     * @brief Start the Finite State Machine.
     */
    void start();

    /**
     * @brief Stop the Finite State Machine.
     */
    void stop();

    /**
     * @brief Update the current state of the FSM.
     */
    void update();

    /**
     * @brief Restart the Finite State Machine on the new cursor position.
     */
    void restart();

    /**
     * @brief Disconnect selected nodes if possible.
     */
    void disconnect_node();

    /**
     * @brief Destructor for DisconnectFSM.
     */
    ~DisconnectFSM() = default;

    /**
     * @brief Get the current data of the FSM.
     * @return Reference to the current FSM data.
     */
    DisconnectData& get_data() { return m_data; }

private:
    DisconnectData m_data;
    DisconnectFSMState* m_current_state = nullptr;
    std::unordered_map<DisconnectState, std::unique_ptr<DisconnectFSMState>> m_states;

    UsdPrimNodeItemBase* m_node;
};

/**
 * @class DisconnectFSMState
 * @brief Base class for Disconnect FSM states.
 */
class DisconnectFSMState
{
public:
    /*
     * @brief Function called when entering this state.
     * @param machine The DisconnectFSM object that is transitioning to this state.
     */
    virtual void enter_state(DisconnectFSM* machine) {}

    /**
     * @brief Function called to update the state of the DisconnectFSM.
     * @param machine The DisconnectFSM object whose state is being updated.
     * @param trigger The trigger that caused the update.
     */
    virtual void update(DisconnectFSM* machine, const DisconnectTrigger& trigger) = 0;

    /**
     * @brief Virtual destructor for DisconnectFSMState.
     */
    virtual ~DisconnectFSMState() = default;
};

/**
 * @class NullState
 * @brief Class representing the Null state of the Disconnect FSM. Associated with DisconnectState::Nulled.
 */
class NullState : public DisconnectFSMState
{
public:
    /*
     * @brief Function called when entering Null state.
     * @param machine The DisconnectFSM object that is transitioning to Null state.
     */
    void enter_state(DisconnectFSM* machine) override;

    /**
     * @brief Function called to update the state of the DisconnectFSM.
     * @param machine The DisconnectFSM object whose state is being updated.
     * @param trigger The trigger that caused the update.
     */
    void update(DisconnectFSM* machine, const DisconnectTrigger& trigger) override;
};

/**
 * @class PressedState
 * @brief Class representing the Pressed state of the Disconnect FSM. Associated with DisconnectState::Pressed.
 */
class PressedState : public DisconnectFSMState
{
public:
    /**
     * @brief Function called to update the state of the DisconnectFSM.
     * @param machine The DisconnectFSM object whose state is being updated.
     * @param trigger The trigger that caused the update.
     */
    void update(DisconnectFSM* machine, const DisconnectTrigger& trigger) override;
};

/**
 * @class StartAreaState
 * @brief Class representing the StartArea state of the Disconnect FSM. Associated with DisconnectState::StartArea.
 */
class StartAreaState : public DisconnectFSMState
{
public:
    /**
     * @brief Function called to update the state of the DisconnectFSM.
     * @param machine The DisconnectFSM object whose state is being updated.
     * @param trigger The trigger that caused the update.
     */
    void update(DisconnectFSM* machine, const DisconnectTrigger& trigger) override;
};

/**
 * @class AfterBorderLineState
 * @brief Class representing the AfterBorderLine state of the Disconnect FSM. Associated with DisconnectState::AfterRightBorder and
 * DisconnectState::AfterLeftBorder.
 */
class AfterBorderLineState : public DisconnectFSMState
{
public:
    /*
     * @brief Constructor for AfterBorderLineState.
     * @param trigger The trigger that caused the transition from this state to the next_state.
     * @param next_state The state that the DisconnectFSM is in after a transitioning triggered by trigger_for_next.
     */
    AfterBorderLineState(DisconnectTrigger trigger_for_next, DisconnectState next_state);

    /**
     * @brief Function called to update the state of the DisconnectFSM.
     * @param machine The DisconnectFSM object whose state is being updated.
     * @param trigger The trigger that caused the update.
     */
    void update(DisconnectFSM* machine, const DisconnectTrigger& trigger) override;

private:
    DisconnectTrigger m_compared_trigger;
    DisconnectState m_state_for_compared_trigger;
};

/**
 * @class AfterChangeDirectionPointState
 * @brief Class representing the AfterChangeDirectionPoint state of the Disconnect FSM. Associated with DisconnectState::AfterRightmostPoint and
 * DisconnectState::AfterLeftmostPoint.
 */
class AfterChangeDirectionPointState : public DisconnectFSMState
{
public:
    /*
     * @brief Constructor for AfterChangeDirectionPointState.
     * @param trigger The trigger that caused the transition from this state to the next_state.
     * @param next_state The state that the DisconnectFSM is in after a transitioning triggered by trigger_for_next.
     * @param is_rightmost True if the change direction point is the rightmost point, false otherwise.
     */
    AfterChangeDirectionPointState(DisconnectTrigger trigger_for_next, DisconnectState next_state, bool is_rightmost = false);

    /*
     * @brief Function called when entering AfterChangeDirectionPoint state.
     * @param machine The DisconnectFSM object that is transitioning to AfterChangeDirectionPoint state.
     */
    void enter_state(DisconnectFSM* machine) override;

    /**
     * @brief Function called to update the state of the DisconnectFSM.
     * @param machine The DisconnectFSM object whose state is being updated.
     * @param trigger The trigger that caused the update.
     */
    void update(DisconnectFSM* machine, const DisconnectTrigger& trigger) override;

private:
    /**
     * @brief Function to check if the current state area contains the cursor position.
     * @param machine A DisconnectFSM object containing data to determine the currently available area.
     * @return True if the cursor position is in the current state area, false otherwise.
     */
    bool in_current_state_area(DisconnectFSM* machine) const;

    bool m_is_rightmost_state;
    DisconnectTrigger m_compared_trigger;
    DisconnectState m_state_for_compared_trigger;
};

/**
 * @class DisconnectedState
 * @brief Class representing the Disconnected state of the Disconnect FSM. Associated with DisconnectState::Nulled.
 */
class DisconnectedState : public DisconnectFSMState
{
public:
    /*
     * @brief Function called when entering Disconnected state.
     * @param machine The DisconnectFSM object that is transitioning to Disconnected state.
     */
    void enter_state(DisconnectFSM* machine) override;

    /**
     * @brief Function called to update the state of the DisconnectFSM.
     * @param machine The DisconnectFSM object whose state is being updated.
     * @param trigger The trigger that caused the update.
     */
    void update(DisconnectFSM* machine, const DisconnectTrigger& trigger) override;
};

OPENDCC_NAMESPACE_CLOSE
