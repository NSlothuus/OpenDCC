/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "api.h"
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"

// #include "opendcc/app/core/api.h"
#include <QtWidgets/QWidget>
#include <memory>
#include <set>
#include <functional>
#include <cfloat>
// #include "opendcc/app/core/settings.h"
#include "opendcc/ui/timeline_widget/time_display.h"

class QDoubleSpinBox;
class QTimer;
class QPushButton;
class QAction;
class QMenu;
class QTimeLine;
class QMediaPlayer;
class QLabel;

OPENDCC_NAMESPACE_OPEN

class TimeWidget;
class FramesPerSecondWidget;
class TimeBarWidget;

enum class TimelineLayout
{
    Default,
    Player
};

/**
 * @brief This class represents a timeline widget for managing and displaying
 *        of frames and keyframes on the timeline.
 */
class OPENDCC_UI_TIMELINE_WIDGET_API TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    /** @brief Enumeration for selecting the playback mode.
     */
    enum class PlaybackMode
    {
        EveryFrame, /**< Playback every individual frame without taking the calculation time into account.*/
        Realtime /**< Playback in real-time, some of the frames may be skipped in order to make an animation played in sync with the actual time. */
    };
    /**
     * @brief Constructs a TimelineWidget object.
     *
     * @param timeline_layout The layout style for the timeline.
     * @param current_time_indicator The current time indicator.
     * @param subdivisions Indicates whether to display subdivisions on the timeline.
     * @param parent The parent QWidget.

     * @note It is important to specify opendcc namespace before CurrentTimeIndicator::Default and TimelineLayout::Default
     * because Shiboken ignores it during generation producing a compilation error.
     */
    TimelineWidget(TimelineLayout timeline_layout = OpenDCC::TimelineLayout::Default,
                   CurrentTimeIndicator current_time_indicator = OpenDCC::CurrentTimeIndicator::Default, bool subdivisions = true,
                   QWidget* parent = nullptr);
    ~TimelineWidget();
    double current_time();
    /**
     * @brief Gets the TimeBarWidget.
     *
     * @return The TimeBarWidget.
     */
    TimeBarWidget* time_bar_widget() { return m_timebar; }
    /**
     * @brief Sets the start time value.
     *
     * @param value The start time value.
     */
    void set_start_time(double value);
    /**
     * @brief Sets the end time value.
     *
     * @param value The end time value.
     */
    void set_end_time(double value);
    /**
     * @brief Checks if the timeline is currently playing.
     *
     * @return True if the timeline is playing, false otherwise.
     */
    bool is_playing() const { return m_is_playing; }
    /**
     * @brief Checks if the timeline is currently scrubbing.
     *
     * @return True if the timeline is scrubbing, false otherwise.
     */
    bool is_scrubbing() const { return !m_is_playing; }
    /**
     * @brief Returns the frames per second (FPS) value.
     *
     * @return The FPS value.
     */
    double get_fps() const { return m_frames_per_second; }

    /**
     * @brief Sets audio file to display and playback using the application timeline.
     * @param filepath The path to a file in the .wav format.
     * @param frame_offset The sound frame offset.
     */
    void set_sound_display(const std::string& filepath, const double frame_offset);

    /**
     * @brief Clears the sound display on timeline and removes file from sound playback.
     */
    void clear_sound_display();

    /**
     * @brief Sets the time display mode.
     *
     * @param mode The time display mode to set.
     */
    void set_time_display(TimeDisplay mode);
    /**
     * @brief Retrieves the current time display mode.
     *
     * @return The current time display mode.
     */
    TimeDisplay get_time_display() const;

    /**
     * @brief Sets the playback mode.
     *
     * @param mode The playback mode to set.
     */
    void set_playback_mode(PlaybackMode mode);
    /**
     * @brief Retrieves the current playback mode.
     *
     * @return The current playback mode.
     */
    PlaybackMode get_playback_mode() const;

    /**
     * @brief Sets the value for the snap playback.
     *
     * @param val The snap playback value.
     *
     * @note The snap playback works only in the "Play Every Frame" mode.
     *
     */
    void set_playback_by(double val);

    /**
     * @brief Retrieves the value for the snap playback.
     *
     * @return The snap playback value.
     *
     * @note The snap playback works only in the "Play Every Frame" mode.
     */
    double get_playback_by() const;
Q_SIGNALS:
    /**
     * @brief Signal emitted when the current time changes.
     *
     * @param value The new current time value.
     */
    void current_time_changed(double value);
    /**
     * @brief Signal emitted when the keyframe draw mode changes.
     */
    void keyframe_draw_mode_changed();
    /**
     * @brief Signal emitted when the time display mode changes.
     *
     * @param mode The new time display mode.
     */
    void time_display_changed(TimeDisplay mode);

public Q_SLOTS:
    /**
     * @brief Steps forward by one keyframe.
     */
    void step_forward_one_key();
    /**
     * @brief Steps backward by one keyframe.
     */
    void step_back_one_key();
    /**
     * @brief Sets the frames per second (FPS) value.
     *
     * @param value The FPS value to set.
     */
    void set_frames_per_second(double value);

    /**
     * @brief Steps forward by one frame.
     */
    void step_forward_one_frame();

    /**
     * @brief Steps backward by one frame.
     */
    void step_back_one_frame();
    /**
     * @brief Jumps to the start of the timeline.
     */
    void go_to_start();
    /**
     * @brief Jumps to the end of the timeline.
     */
    void go_to_end();

    /**
     * @brief Starts playing forwards.
     */
    void play_forwards();
    /**
     * @brief Starts playing backwards.
     */
    void play_backwards();
    /**
     * @brief Stops the playback.
     */
    void stop_play();

private Q_SLOTS:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void update_time_edit(double value);
    void time_spinbox_editing_finished();
    void time_drag(double time);

private:
    QTimer* m_timer;
    QTimeLine* m_timeline;

    PlaybackMode m_playback_mode = PlaybackMode::EveryFrame;

    TimelineLayout m_timeline_layout;

    TimeWidget* m_current_time_edit;
    TimeBarWidget* m_timebar;
    QMenu* m_context_menu;

    QLabel* m_time_display_label;

    QPushButton* m_play_backward_btn;
    QPushButton* m_play_forward_btn;
    QAction* m_step_forward_one_key_act;
    QAction* m_step_back_one_key_act;
    bool m_is_play_forward;
    bool m_is_playing = false;
    double m_frames_per_second = 24.0;
    double m_playback_by = 1.0;

    bool m_sound = false;
    bool m_sound_playing = false;
    double m_sound_start = 0.0;
    double m_sound_frame = -DBL_MAX;
    QMediaPlayer* m_player;
    QTimer* m_drag_timer;

    TimeDisplay m_time_display = TimeDisplay::Frames;
};

OPENDCC_NAMESPACE_CLOSE
