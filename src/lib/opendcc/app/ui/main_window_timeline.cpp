// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/application.h"
#include "opendcc/app/ui/main_window.h"
#include "opendcc/ui/timeline_widget/timebar_widget.h"
#include "opendcc/ui/timeline_widget/timeline_widget.h"
#include "opendcc/app/core/undo/block.h"
#include "opendcc/app/ui/application_ui.h"

#include <pxr/usd/sdf/timeCode.h>

#include <QSettings>
#include <QToolBar>
#include <QSignalBlocker>

OPENDCC_NAMESPACE_OPEN

void MainWindow::update_timeline_samples()
{
    if (!m_timeline_widget || !m_timeline_widget->time_bar_widget())
        return;

    if (m_timeline_widget->time_bar_widget()->get_keyframe_draw_mode() != KeyframeDrawMode::Timesamples)
        return;
    auto stage = Application::instance().get_session()->get_current_stage();
    KeyFrameSet times_set;
    if (stage)
    {
        auto prim_paths = Application::instance().get_prim_selection();
        std::vector<PXR_NS::UsdAttributeQuery> attr_query_list;
        for (auto prim_path : prim_paths)
        {
            auto prim = stage->GetPrimAtPath(prim_path);
            if (prim)
            {
                auto attrs = prim.GetAuthoredAttributes();
                for (auto attr : attrs)
                {
                    attr_query_list.push_back(PXR_NS::UsdAttributeQuery(attr));
                }
            }
        }
        std::vector<double> times;
        PXR_NS::GfInterval frame_range(stage->GetStartTimeCode(), stage->GetEndTimeCode());
        PXR_NS::UsdAttributeQuery::GetUnionedTimeSamplesInInterval(attr_query_list, frame_range, &times);

        for (auto time : times)
        {
            times_set.insert(time);
        }
    }
    m_timeline_widget->time_bar_widget()->set_keyframes(times_set);
}

// void MainWindow::init_timeline_ui(TimelineWidget* timeline_widget)
void MainWindow::init_timeline_ui()
{
    // m_timeline_widget = timeline_widget;
    m_timeline_widget = new TimelineWidget(TimelineLayout::Default, CurrentTimeIndicator::Default,
                                           true, // subdivisions
                                           this); // parent

    // RangeSlider
    m_timeline_slider = new TimelineSlider(this);
    m_timeline_slider->set_fps(m_timeline_widget->get_fps());
    m_timeline_slider->set_time_display(m_timeline_widget->get_time_display());

    QToolBar* range_slider_toolbar = new QToolBar(i18n("toolbars.timeline_slider", "Timeline Slider"));
    range_slider_toolbar->setObjectName("timeline_slider");
    range_slider_toolbar->setProperty("opendcc_toolbar", true);
    range_slider_toolbar->setProperty("opendcc_toolbar_side", "bottom");
    range_slider_toolbar->setProperty("opendcc_toolbar_row", 0);
    range_slider_toolbar->setProperty("opendcc_toolbar_index", 0);

    range_slider_toolbar->addWidget(m_timeline_slider);

    QToolBar* timeline_toolbar = new QToolBar(i18n("toolbars.timeline_slider", "Timeline"));
    timeline_toolbar->setObjectName("timeline_toolbar");
    timeline_toolbar->setProperty("opendcc_toolbar", true);
    timeline_toolbar->setProperty("opendcc_toolbar_side", "bottom");
    timeline_toolbar->setProperty("opendcc_toolbar_row", 1);
    timeline_toolbar->setProperty("opendcc_toolbar_index", 0);
    timeline_toolbar->addWidget(m_timeline_widget);

    addToolBar(Qt::ToolBarArea::BottomToolBarArea, range_slider_toolbar);
    addToolBarBreak(Qt::ToolBarArea::BottomToolBarArea);
    addToolBar(Qt::ToolBarArea::BottomToolBarArea, timeline_toolbar);

    // something is very wrong here
    // const bool timeline_snap_value = m_settings->value("timeline.snap", true).toBool();
    // m_timeline_widget->time_bar_widget()->set_snap_time_mode(timeline_snap_value);

    connect(m_timeline_widget, &TimelineWidget::current_time_changed, [&](double time) { Application::instance().set_current_time(time); });

    m_timeline_selection_changed_callback_id =
        Application::instance().register_event_callback(Application::EventType::SELECTION_CHANGED, [&]() { update_timeline_samples(); });
    m_timeline_current_time_changed_callback_id =
        Application::instance().register_event_callback(Application::EventType::CURRENT_TIME_CHANGED, [&]() {
            QSignalBlocker timeline_widget_blocker(m_timeline_widget);
            m_timeline_widget->time_bar_widget()->set_current_time(Application::instance().get_current_time());
        });
    connect(m_timeline_widget, &TimelineWidget::keyframe_draw_mode_changed, [&]() { update_timeline_samples(); });

    m_escape_action_callback_id = Application::instance().register_event_callback(Application::EventType::UI_ESCAPE_KEY_ACTION, [this]() {
        if (m_timeline_widget)
        {
            if (m_timeline_widget->is_playing())
                m_timeline_widget->stop_play();
        }
    });

    TimeBarWidget* time_bar_widget = m_timeline_widget->time_bar_widget();
    RangeSlider* range_slider = m_timeline_slider->get_range_slider();

    range_slider->set_start_time(time_bar_widget->start_time());
    range_slider->set_current_start_time(time_bar_widget->start_time());
    range_slider->set_end_time(time_bar_widget->end_time());
    range_slider->set_current_end_time(time_bar_widget->end_time());

    QObject::connect(m_timeline_widget, &TimelineWidget::time_display_changed, m_timeline_slider, &TimelineSlider::set_time_display);
    QObject::connect(m_timeline_slider, &TimelineSlider::fps_changed, m_timeline_widget, &TimelineWidget::set_frames_per_second);
    QObject::connect(m_timeline_slider, &TimelineSlider::fps_changed, [&](double fps) {
        auto stage = Application::instance().get_session()->get_current_stage();
        if (stage && !PXR_NS::GfIsClose(stage->GetFramesPerSecond(), fps, 0.00001))
        {
            commands::UsdEditsUndoBlock block;
            stage->SetFramesPerSecond(fps);
        }
    });

    QObject::connect(range_slider, &RangeSlider::current_start_time_changed, m_timeline_widget, &TimelineWidget::set_start_time);
    QObject::connect(range_slider, &RangeSlider::current_end_time_changed, m_timeline_widget, &TimelineWidget::set_end_time);

    QObject::connect(range_slider, &RangeSlider::end_time_changed, [this](double time) {
        auto stage = Application::instance().get_session()->get_current_stage();

        if (stage && !m_timeline_slider->get_range_slider()->slider_moving() && !qFuzzyCompare(stage->GetEndTimeCode(), time))
        {
            commands::UsdEditsUndoBlock block;
            stage->SetEndTimeCode(time);
        }
    });

    QObject::connect(range_slider, &RangeSlider::start_time_changed, [this](double time) {
        auto stage = Application::instance().get_session()->get_current_stage();

        if (stage && !m_timeline_slider->get_range_slider()->slider_moving() && !qFuzzyCompare(stage->GetStartTimeCode(), time))
        {
            commands::UsdEditsUndoBlock block;
            stage->SetStartTimeCode(time);
        }
    });

    QObject::connect(range_slider, &RangeSlider::current_start_time_changed, [this](double time) {
        auto stage = Application::instance().get_session()->get_current_stage();

        if (!stage)
        {
            return;
        }

        RangeSlider* range_slider = m_timeline_slider->get_range_slider();

        const auto min_time_code_token = PXR_NS::TfToken("minTimeCode");

        bool has_authored_min_time_code = stage->HasAuthoredMetadata(min_time_code_token);

        bool new_value = !has_authored_min_time_code && !qFuzzyCompare(range_slider->get_current_start_time(), range_slider->get_start_time());

        PXR_NS::SdfTimeCode min_time_code;
        bool found = stage->GetMetadata(min_time_code_token, &min_time_code);

        bool change_value = has_authored_min_time_code && found && !qFuzzyCompare(range_slider->get_current_start_time(), min_time_code.GetValue());

        if (!range_slider->slider_moving() && (new_value || change_value))
        {
            commands::UsdEditsUndoBlock block;
            stage->SetMetadata(min_time_code_token, PXR_NS::SdfTimeCode(time));
        }
    });

    QObject::connect(range_slider, &RangeSlider::current_end_time_changed, [this](double time) {
        auto stage = Application::instance().get_session()->get_current_stage();

        if (!stage)
        {
            return;
        }

        RangeSlider* range_slider = m_timeline_slider->get_range_slider();

        const auto max_time_code_token = PXR_NS::TfToken("maxTimeCode");

        bool has_authored_max_time_code = stage->HasAuthoredMetadata(max_time_code_token);

        bool new_value = !has_authored_max_time_code && !qFuzzyCompare(range_slider->get_current_end_time(), range_slider->get_end_time());

        PXR_NS::SdfTimeCode max_time_code;
        bool found = stage->GetMetadata(max_time_code_token, &max_time_code);

        bool change_value = has_authored_max_time_code && found && !qFuzzyCompare(range_slider->get_current_start_time(), max_time_code.GetValue());

        if (!range_slider->slider_moving() && (new_value || change_value))
        {
            commands::UsdEditsUndoBlock block;
            stage->SetMetadata(max_time_code_token, PXR_NS::SdfTimeCode(time));
        }
    });

    QObject::connect(range_slider, &RangeSlider::range_changed, [this](double start_time, double end_time) {
        auto stage = Application::instance().get_session()->get_current_stage();

        if (!stage)
        {
            return;
        }

        RangeSlider* range_slider = m_timeline_slider->get_range_slider();

        const auto max_time_code_token = PXR_NS::TfToken("maxTimeCode");
        const auto min_time_code_token = PXR_NS::TfToken("minTimeCode");

        PXR_NS::SdfTimeCode max_time_code;
        PXR_NS::SdfTimeCode min_time_code;

        const bool found_max = stage->GetMetadata(max_time_code_token, &max_time_code);
        const bool found_min = stage->GetMetadata(min_time_code_token, &min_time_code);

        if (!range_slider->slider_moving() && (!found_max || !qFuzzyCompare(max_time_code.GetValue(), range_slider->get_current_end_time()) ||
                                               !found_min || !qFuzzyCompare(min_time_code.GetValue(), range_slider->get_current_start_time())))
        {
            commands::UsdEditsUndoBlock block;
            stage->SetMetadata(min_time_code_token, PXR_NS::SdfTimeCode(start_time));
            stage->SetMetadata(max_time_code_token, PXR_NS::SdfTimeCode(end_time));
        }
    });

    auto settings = Application::instance().get_settings();
    m_timeline_widget->set_playback_by(settings->get("timeline.playback_by", 1.0));
    m_timeline_playback_by_callback_id = Application::instance().get_settings()->register_setting_changed(
        "timeline.playback_by", [this](const Settings::Value& val) { m_timeline_widget->set_playback_by(val.get(1.0)); });
    m_timeline_widget->set_playback_mode(
        (TimelineWidget::PlaybackMode)settings->get("timeline.playback_mode", (int)TimelineWidget::PlaybackMode::EveryFrame));
    m_timeline_playback_mode_callback_id =
        Application::instance().get_settings()->register_setting_changed("timeline.playback_mode", [this](const Settings::Value& val) {
            m_timeline_widget->set_playback_mode(static_cast<TimelineWidget::PlaybackMode>(val.get<int>(0)));
        });
    m_timeline_widget->time_bar_widget()->set_snap_time_mode(settings->get("timeline.snap", true));
    m_timeline_snap_callback_id = Application::instance().get_settings()->register_setting_changed(
        "timeline.snap", [this](const Settings::Value& val) { m_timeline_widget->time_bar_widget()->set_snap_time_mode(val.get<bool>(true)); });

    m_timeline_widget->time_bar_widget()->set_current_time_indicator_type(
        settings->get("timeline.current_time_indicator", 0) == 0 ? CurrentTimeIndicator::Default : CurrentTimeIndicator::Arrow);
    m_timeline_keyframe_current_time_indicator_type_callback_id =
        Application::instance().get_settings()->register_setting_changed("timeline.current_time_indicator", [this](const Settings::Value& val) {
            m_timeline_widget->time_bar_widget()->set_current_time_indicator_type(val.get<int>(0) == 0 ? CurrentTimeIndicator::Default
                                                                                                       : CurrentTimeIndicator::Arrow);
        });

    auto get_keyframe_display_type = [](int display_type) {
        switch (display_type)
        {
        case 0:
            return KeyframeDisplayType::Line;
        case 1:
            return KeyframeDisplayType::Rect;
        case 2:
            return KeyframeDisplayType::Arrow;

        default:
            return KeyframeDisplayType::Line;
        }
    };

    m_timeline_widget->time_bar_widget()->set_keyframe_display_type(get_keyframe_display_type(settings->get("timeline.keyframe_display_type", 0)));
    m_timeline_keyframe_display_type_callback_id = Application::instance().get_settings()->register_setting_changed(
        "timeline.keyframe_display_type", [this, get_keyframe_display_type](const Settings::Value& val) {
            m_timeline_widget->time_bar_widget()->set_keyframe_display_type(get_keyframe_display_type(val.get<int>(0)));
        });

    m_timeline_stage_callback_id = Application::instance().get_session()->register_stage_changed_callback(
        Session::StageChangedEventType::CURRENT_STAGE_OBJECT_CHANGED, [this](PXR_NS::UsdNotice::ObjectsChanged const& notice) {
            auto notice_stage = notice.GetStage();
            auto current_stage = Application::instance().get_session()->get_current_stage();

            if (notice_stage != current_stage)
            {
                return;
            }

            RangeSlider* range_slider = m_timeline_slider->get_range_slider();

            bool found_start_time = false;
            bool found_end_time = false;
            double start_time;
            double end_time;

            for (auto& path : notice.GetChangedInfoOnlyPaths())
            {
                if (path == PXR_NS::SdfPath("/"))
                {
                    for (auto& token : notice.GetChangedFields(path))
                    {
                        if (token == PXR_NS::TfToken("startTimeCode") && current_stage->HasAuthoredMetadata(token) &&
                            !qFuzzyCompare(range_slider->get_start_time(), current_stage->GetStartTimeCode()))
                        {
                            m_timeline_slider->get_range_slider()->set_start_time(current_stage->GetStartTimeCode());
                        }
                        else if (token == PXR_NS::TfToken("endTimeCode") && current_stage->HasAuthoredMetadata(token) &&
                                 !qFuzzyCompare(range_slider->get_end_time(), current_stage->GetEndTimeCode()))
                        {
                            m_timeline_slider->get_range_slider()->set_end_time(current_stage->GetEndTimeCode());
                        }
                        else if (token == PXR_NS::TfToken("minTimeCode") && current_stage->HasAuthoredMetadata(token))
                        {
                            PXR_INTERNAL_NS::SdfTimeCode code;

                            if (!current_stage->GetMetadata(token, &code))
                            {
                                break;
                            }

                            start_time = code.GetValue();
                            found_start_time = true;
                        }
                        else if (token == PXR_NS::TfToken("maxTimeCode") && current_stage->HasAuthoredMetadata(token))
                        {
                            PXR_INTERNAL_NS::SdfTimeCode code;

                            if (!current_stage->GetMetadata(token, &code))
                            {
                                break;
                            }

                            end_time = code.GetValue();
                            found_end_time = true;
                        }
                    }

                    break;
                }
            }

            if (found_start_time && !qFuzzyCompare(start_time, range_slider->get_current_start_time()))
            {
                range_slider->set_current_start_time(start_time);
            }
            else if (!current_stage->HasAuthoredMetadata(PXR_NS::TfToken("minTimeCode")))
            {
                range_slider->set_current_start_time(range_slider->get_start_time());
            }

            if (found_end_time && !qFuzzyCompare(end_time, range_slider->get_current_end_time()))
            {
                range_slider->set_current_end_time(end_time);
            }
            else if (!current_stage->HasAuthoredMetadata(PXR_NS::TfToken("maxTimeCode")))
            {
                range_slider->set_current_end_time(range_slider->get_end_time());
            }

            if (!qFuzzyCompare(m_timeline_widget->get_fps(), current_stage->GetFramesPerSecond()))
            {
                m_timeline_widget->set_frames_per_second(current_stage->GetFramesPerSecond());
                m_timeline_slider->set_fps(current_stage->GetFramesPerSecond());
            }
        });

    m_timeline_current_stage_callback_id = Application::instance().register_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, [this]() {
        auto stage = Application::instance().get_session()->get_current_stage();
        if (stage)
        {
            RangeSlider* range_slider = m_timeline_slider->get_range_slider();

            const QSignalBlocker blocker(range_slider);

            const bool end_time = stage->HasAuthoredMetadata(PXR_NS::TfToken("endTimeCode"));
            const bool start_time = stage->HasAuthoredMetadata(PXR_NS::TfToken("startTimeCode"));

            const auto end_time_code = stage->GetEndTimeCode();
            const auto start_time_code = stage->GetStartTimeCode();

            const auto max_time_code_token = PXR_NS::TfToken("maxTimeCode");
            const auto min_time_code_token = PXR_NS::TfToken("minTimeCode");

            if (end_time)
            {
                m_timeline_widget->set_end_time(end_time_code);
                range_slider->set_end_time(end_time_code);
                range_slider->set_current_end_time(end_time_code);
            }
            if (start_time)
            {
                m_timeline_widget->set_start_time(start_time_code);
                range_slider->set_start_time(start_time_code);
                range_slider->set_current_start_time(start_time_code);
            }

            if (stage->HasAuthoredMetadata(max_time_code_token))
            {
                PXR_NS::SdfTimeCode max_time_code;
                const bool found_max = stage->GetMetadata(max_time_code_token, &max_time_code);

                range_slider->set_current_end_time(found_max ? max_time_code.GetValue() : end_time_code);
            }

            if (stage->HasAuthoredMetadata(min_time_code_token))
            {
                PXR_NS::SdfTimeCode min_time_code;
                const bool found_min = stage->GetMetadata(min_time_code_token, &min_time_code);

                range_slider->set_current_start_time(found_min ? min_time_code.GetValue() : start_time_code);
            }

            if (stage->HasAuthoredMetadata(PXR_NS::TfToken("framesPerSecond")))
            {
                const auto fps = stage->GetFramesPerSecond();

                if (!qFuzzyCompare(m_timeline_widget->get_fps(), fps))
                {
                    m_timeline_widget->set_frames_per_second(fps);
                    m_timeline_slider->set_fps(fps);
                }
            }

            m_timeline_slider->update_time_widgets(0);
        }
    });
}

void MainWindow::cleanup_timeline_ui()
{
    Application::instance().get_session()->unregister_stage_changed_callback(Session::StageChangedEventType::CURRENT_STAGE_OBJECT_CHANGED,
                                                                             m_timeline_stage_callback_id);
    Application::instance().unregister_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, m_timeline_current_stage_callback_id);
    Application::instance().unregister_event_callback(Application::EventType::SELECTION_CHANGED, m_timeline_selection_changed_callback_id);
    Application::instance().unregister_event_callback(Application::EventType::UI_ESCAPE_KEY_ACTION, m_escape_action_callback_id);
    Application::instance().get_settings()->unregister_setting_changed("timeline.playback_by", m_timeline_playback_by_callback_id);
    Application::instance().get_settings()->unregister_setting_changed("timeline.playback_mode", m_timeline_playback_mode_callback_id);
    Application::instance().get_settings()->unregister_setting_changed("timeline.snap", m_timeline_snap_callback_id);
}

OPENDCC_NAMESPACE_CLOSE
