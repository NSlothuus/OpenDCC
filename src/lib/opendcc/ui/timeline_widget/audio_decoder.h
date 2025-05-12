/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <QObject>
#include <QAudioDecoder>

/**
 * @brief This class represents an audio decoder with waveform analysis capabilities.
 */

class AudioDecoder : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an AudioDecoder.
     *
     * @param parent The parent QObject.
     */
    AudioDecoder(QObject* parent = nullptr);
    /**
     * @brief Sets the source filename for decoding.
     *
     * @param path The path of the audio file to be decoded.
     */
    void set_source_filename(const QString& path);

    /**
     * @brief Retrieves the audio levels.
     *
     * @return The audio levels.
     */
    const QVector<qreal>& get_levels() const;
    /**
     * @brief Checks if the decoder is ready for decoding.
     *
     * @return True if the decoder is ready, false otherwise.
     */
    const bool is_ready() const;
    /**
     * @brief Computes the waveform from the audio samples based on the given parameters.
     *
     * @param wave The computed waveform.
     * @param num_pixels The number of pixels to represent the waveform.
     * @param start_frame The starting frame.
     * @param end_frame The ending frame.
     * @param fps The frames per second.
     * @param play_frame The current frame for playback.
     */
    void compute_wave(QVector<qreal>& wave, const int num_pixels, const double start_frame, const double end_frame, const double fps,
                      const double play_frame);
    /**
     * @brief Resets the decoder and clears any stored data.
     */
    void clear();

Q_SIGNALS:
    /**
     * @brief Signal emitted when decoding is finished.
     */
    void finish_decoding();

private Q_SLOTS:
    void process_buffer();
    void finish();
    void handle_error(QAudioDecoder::Error error);

private:
    bool m_ready = false;
    QAudioDecoder* m_audio_decoder;
    QAudioBuffer m_audio_buffer;
    QVector<qreal> m_levels;
};
