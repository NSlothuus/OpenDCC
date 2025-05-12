// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "opendcc/opendcc.h"
#include <QtCore/QThread>

#include <string>

OPENDCC_NAMESPACE_OPEN

class RenderViewMainWindow;

class RenderViewListenerThread : public QThread
{
    Q_OBJECT

    void run();

public:
    RenderViewListenerThread(RenderViewMainWindow* app, void* zmq_ctx)
        : m_app(app)
        , m_zmq_ctx(zmq_ctx) {};

private:
    RenderViewMainWindow* m_app;
    void* m_zmq_ctx;
Q_SIGNALS:
    void new_image();
};

OPENDCC_NAMESPACE_CLOSE
