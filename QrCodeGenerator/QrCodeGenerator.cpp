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

#include "QrCodeGenerator.h"

#include <sstream>
#include <string>

#include <QPainter>
#include <QSvgRenderer>

QrCodeGenerator::QrCodeGenerator(QObject *parent) : QObject(parent) {}

QImage QrCodeGenerator::generateQr(const QString &data, const quint16 size,
                                   const quint16 borderSize,
                                   qrcodegen::QrCode::Ecc errorCorrection) {
  QByteArray byteArray = data.toUtf8();
  const qrcodegen::QrCode qrCode = qrcodegen::QrCode::encodeText(byteArray.constData(),
                                                                 errorCorrection);
  return qrCodeToImage(qrCode, borderSize, size);
}

QString QrCodeGenerator::toSvgString(const qrcodegen::QrCode &qr, quint16 border) const {
  if (border > INT_MAX / 2 || border * 2 > INT_MAX - qr.getSize())
    throw std::overflow_error("Border too large");

  std::ostringstream sb;
  sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
        "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
  sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 ";
  sb << (qr.getSize() + border * 2) << " " << (qr.getSize() + border * 2)
     << "\" stroke=\"none\">\n";
  sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
  sb << "\t<path d=\"";
  for (int y = 0; y < qr.getSize(); y++) {
    for (int x = 0; x < qr.getSize(); x++) {
      if (qr.getModule(x, y)) {
        if (x != 0 || y != 0)
          sb << " ";
        sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
      }
    }
  }
  sb << "\" fill=\"#000000\"/>\n";
  sb << "</svg>\n";

  return QString::fromStdString(sb.str());
}

QImage QrCodeGenerator::qrCodeToImage(const qrcodegen::QrCode &qrCode, quint16 border,
                                      const quint16 size) const {
  auto svg = toSvgString(qrCode, border);
  QSvgRenderer render(svg.toUtf8());
  QImage image(size, size, QImage::Format_Mono);
  image.fill(Qt::white);
  QPainter painter(&image);
  render.render(&painter);
  return image;
}
