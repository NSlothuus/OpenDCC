// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/usd_node_editor/node_disconnect_machine.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"

OPENDCC_NAMESPACE_OPEN
const double s_disconnect_offset = 160.0;
const int s_disconnect_time = 500;

//---------------------------------------------------------------------------------------------------------------------------------------------------
// class DisconnectMachine
//---------------------------------------------------------------------------------------------------------------------------------------------------

DisconnectFSM::DisconnectFSM(UsdPrimNodeItemBase* node)
    : m_node(node)
{
    m_states[DisconnectState::Nulled] = std::make_unique<NullState>();
    m_states[DisconnectState::Pressed] = std::make_unique<PressedState>();
    m_states[DisconnectState::StartArea] = std::make_unique<StartAreaState>();

    m_states[DisconnectState::AfterRightBorder] =
        std::make_unique<AfterBorderLineState>(DisconnectTrigger::DirectionChangeAtRightmostPoint, DisconnectState::AfterRightmostPoint);
    m_states[DisconnectState::AfterLeftBorder] =
        std::make_unique<AfterBorderLineState>(DisconnectTrigger::DirectionChangeAtLeftmostPoint, DisconnectState::AfterLeftmostPoint);

    m_states[DisconnectState::AfterRightmostPoint] =
        std::make_unique<AfterChangeDirectionPointState>(DisconnectTrigger::RightToLeft, DisconnectState::AfterLeftBorder, true);
    m_states[DisconnectState::AfterLeftmostPoint] =
        std::make_unique<AfterChangeDirectionPointState>(DisconnectTrigger::LeftToRight, DisconnectState::AfterRightBorder, false);

    m_states[DisconnectState::Disconnected] = std::make_unique<DisconnectedState>();

    m_current_state = m_states[DisconnectState::Nulled].get();

    m_data.timer = new QTimer(node);
    m_data.timer->setInterval(s_disconnect_time);
    m_data.timer->callOnTimeout([this] {
        bool is_null_state = m_current_state == m_states[DisconnectState::Nulled].get();
        bool is_disconnected = m_current_state == m_states[DisconnectState::Disconnected].get();
        if (is_null_state || is_disconnected)
        {
            return;
        }
        restart();
    });
}

void DisconnectFSM::start()
{
    m_current_state->update(this, DisconnectTrigger::Press);
}

void DisconnectFSM::stop()
{
    m_current_state->update(this, DisconnectTrigger::Release);
}

void DisconnectFSM::restart()
{
    auto new_start = m_data.get_current_x();
    stop();
    m_data.set_start_x(new_start);
    start();
}

void DisconnectFSM::disconnect_node()
{
    UsdPrimNodeItemBase::InsertionData data;
    if (m_node->can_disconnect_after_shake(data))
    {
        m_node->cut_node_from_connection(data);
        m_node->set_disconnect_mode(true);
    }
}

void DisconnectFSM::update()
{
    m_current_state->update(this, DisconnectTrigger::ToStart);
    m_current_state->update(this, DisconnectTrigger::LeftStartArea);

    if (m_data.is_start_move_to_right())
    {
        m_current_state->update(this, DisconnectTrigger::DirectionChangeAtRightmostPoint);
        m_current_state->update(this, DisconnectTrigger::RightToLeft);
        m_current_state->update(this, DisconnectTrigger::DirectionChangeAtLeftmostPoint);
    }
    else
    {
        m_current_state->update(this, DisconnectTrigger::DirectionChangeAtLeftmostPoint);
        m_current_state->update(this, DisconnectTrigger::LeftToRight);
        m_current_state->update(this, DisconnectTrigger::DirectionChangeAtRightmostPoint);
    }
    m_current_state->update(this, DisconnectTrigger::Disconnect);
}

void DisconnectFSM::set_state(DisconnectState new_state)
{
    m_current_state = m_states[new_state].get();
    m_current_state->enter_state(this);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------
// struct DisconnectData
//---------------------------------------------------------------------------------------------------------------------------------------------------

void DisconnectData::clear_data()
{
    m_start_x = 0;
    m_start_move_to_right = false;

    m_current_x = 0;
    m_old_x = 0;
    m_old_old_x = 0;

    m_rightmost_x = 0;
    m_leftmost_x = 0;

    if (timer->remainingTime() > 0)
    {
        timer->stop();
    }
}

void DisconnectData::set_current_x(int x)
{
    m_old_old_x = m_old_x;
    m_old_x = m_current_x;
    m_current_x = x;
}

void DisconnectData::set_start_x(int x)
{
    m_start_x = x;
    m_current_x = x;
    m_old_x = x;
    m_old_old_x = x;
}

void DisconnectData::set_start_direction(bool is_right)
{
    m_start_move_to_right = is_right;
    timer->start();
}

void DisconnectData::remember_change_direction_x(bool is_rightmost)
{
    if (is_rightmost)
    {
        m_rightmost_x = m_current_x;
    }
    else
    {
        m_leftmost_x = m_current_x;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------
// class NullState
//---------------------------------------------------------------------------------------------------------------------------------------------------

void NullState::enter_state(DisconnectFSM* machine)
{
    machine->get_data().clear_data();
}

void NullState::update(DisconnectFSM* machine, const DisconnectTrigger& trigger)
{
    if (trigger == DisconnectTrigger::Press)
    {
        machine->set_state(DisconnectState::Pressed);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------
// class PressedState
//---------------------------------------------------------------------------------------------------------------------------------------------------

void PressedState::update(DisconnectFSM* machine, const DisconnectTrigger& trigger)
{
    if (trigger == DisconnectTrigger::ToStart)
    {
        machine->set_state(DisconnectState::StartArea);
    }
    else if (trigger == DisconnectTrigger::Release)
    {
        machine->set_state(DisconnectState::Nulled);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------
// class StartAreaState
//---------------------------------------------------------------------------------------------------------------------------------------------------

void StartAreaState::update(DisconnectFSM* machine, const DisconnectTrigger& trigger)
{
    if (trigger == DisconnectTrigger::Release)
    {
        machine->set_state(DisconnectState::Nulled);
        return;
    }

    if (trigger != DisconnectTrigger::LeftStartArea)
    {
        return;
    }

    bool on_the_right = machine->get_data().get_current_x() > machine->get_data().get_start_x() + s_disconnect_offset;
    bool on_the_left = machine->get_data().get_current_x() < machine->get_data().get_start_x() - s_disconnect_offset;
    if (on_the_right || on_the_left)
    {
        machine->get_data().set_start_direction(on_the_right);
        machine->set_state(on_the_right ? DisconnectState::AfterRightBorder : DisconnectState::AfterLeftBorder);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------
// class AfterBorderLineState
//---------------------------------------------------------------------------------------------------------------------------------------------------

AfterBorderLineState::AfterBorderLineState(DisconnectTrigger trigger, DisconnectState state)
    : m_compared_trigger(trigger)
    , m_state_for_compared_trigger(state)
{
}

void AfterBorderLineState::update(DisconnectFSM* machine, const DisconnectTrigger& trigger)
{
    if (trigger == DisconnectTrigger::Release)
    {
        machine->set_state(DisconnectState::Nulled);
        return;
    }

    if (trigger != m_compared_trigger)
    {
        return;
    }
    if (machine->get_data().is_direction_changed())
    {
        machine->set_state(m_state_for_compared_trigger);
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------
// class AfterChangeDirectionPointState
//---------------------------------------------------------------------------------------------------------------------------------------------------

AfterChangeDirectionPointState::AfterChangeDirectionPointState(DisconnectTrigger trigger, DisconnectState state, bool is_rightmost /* = false*/)
    : m_compared_trigger(trigger)
    , m_state_for_compared_trigger(state)
    , m_is_rightmost_state(is_rightmost)
{
}

void AfterChangeDirectionPointState::enter_state(DisconnectFSM* machine)
{
    machine->get_data().remember_change_direction_x(m_is_rightmost_state);
}

void AfterChangeDirectionPointState::update(DisconnectFSM* machine, const DisconnectTrigger& trigger)
{
    if (trigger == DisconnectTrigger::Release)
    {
        machine->set_state(DisconnectState::Nulled);
        return;
    }

    if (in_current_state_area(machine))
    {
        return;
    }

    if (trigger == DisconnectTrigger::Disconnect)
    {
        machine->set_state(DisconnectState::Disconnected);
    }
    else if (trigger == m_compared_trigger)
    {
        machine->set_state(m_state_for_compared_trigger);
    }
}

bool AfterChangeDirectionPointState::in_current_state_area(DisconnectFSM* machine) const
{
    if (m_is_rightmost_state)
    {
        return machine->get_data().get_current_x() > machine->get_data().get_rightmost_x() - s_disconnect_offset;
    }
    return machine->get_data().get_current_x() < machine->get_data().get_leftmost_x() + s_disconnect_offset;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------
// class DisconnectededState
//---------------------------------------------------------------------------------------------------------------------------------------------------

void DisconnectedState::enter_state(DisconnectFSM* machine)
{
    machine->disconnect_node();
}

void DisconnectedState::update(DisconnectFSM* machine, const DisconnectTrigger& trigger)
{
    if (trigger == DisconnectTrigger::Release)
    {
        machine->set_state(DisconnectState::Nulled);
    }
}

OPENDCC_NAMESPACE_CLOSE
