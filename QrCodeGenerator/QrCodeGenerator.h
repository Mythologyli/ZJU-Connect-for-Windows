/*
 * Copyright (c) 2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QImage>
#include <QObject>
#include <QString>

#include "qrcodegen/qrcodegen.h"

/**
 * @class QrCodeGenerator
 * @brief The QrCodeGenerator class is a simple C++ class that uses the qrcodegen library to
 *        generate QR codes from QStrings in Qt applications.
 */
class QrCodeGenerator : public QObject {
public:
  /**
   * @brief Constructs a QrCodeGenerator object.
   * @param parent The parent QObject.
   */
  explicit QrCodeGenerator(QObject *parent = nullptr);

  /**
   * @brief Generates a QR code from the given data and error correction level.
   * @param data The QString containing the data to encode in the QR code.
   * @param size The desired width/height of the generated image (default: 500).
   * @param borderSize The desired border width of the generated image (default: 1).
   * @param errorCorrection The desired error correction level (default:
   * qrcodegen::QrCode::Ecc::MEDIUM).
   *
   * @return QImage containing the generated QR code.
   */
  QImage generateQr(const QString &data, const quint16 size = 500, const quint16 borderSize = 1,
                    qrcodegen::QrCode::Ecc errorCorrection = qrcodegen::QrCode::Ecc::MEDIUM);

private:
  /**
   * @brief Converts a qrcodegen::QrCode object to a SVG image.
   * @param qrCode The qrcodegen::QrCode object to convert.
   * @param borderSize The desired border width of the generated image (default: 1).
   *
   * @return QImage containing the QR code.
   */
  QString toSvgString(const qrcodegen::QrCode &qr, quint16 border) const;

  /**
   * @brief Converts a qrcodegen::QrCode object to a QImage.
   * @param qrCode The qrcodegen::QrCode object to convert.
   * @param size The desired width/height of the generated image.
   * @param borderSize The desired border width of the generated image.
   *
   * @return QImage containing the QR code.
   */
  QImage qrCodeToImage(const qrcodegen::QrCode &qrCode, quint16 border, const quint16 size) const;
};
