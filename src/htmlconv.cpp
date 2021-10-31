#include "config.h"
#include <iostream>
#include <exception>
#include <stdexcept>
#include <QApplication>
#include <QFile>
#include <QList>
#include <QPair>
#include <QPrinter>
#include <QPainter>
#include <QImage>
#include <QEventLoop>
#include <QScopedPointer>
#include <arx/Foreach.h>
#include <arx/Utility.h> /* For unreachable. */
#include "htmlconv/HtmlConv.h"

enum InstructionType {
  PPI,         /**< Pixels-per-inch instruction, changes PPI for the next page. */
  ORIENTATION, /**< Orientation instruction, changes orientation for the next page. */
  INPUT        /**< Input instruction, renders a page. */
};

typedef QPair<InstructionType, QVariant> Instruction;

int main(int argc, char** argv) {
  /* Name of the output file (PDF or image). */
  QString outputFileName;

  /* Width & height of the output image (if rendering into an image). */
  int imageWidth = 0;
  int imageHeight = 0;

  /* Image scale. Used only when rendering into an image. */
  double scale = 0.0;

  /* List of instructions. */
  QList<Instruction> instructions;

  /* Use antialiasing? Is meaningful only for image output. */
  bool antialising = false;

  try {
    QApplication app(argc, argv);

    /* 
     * Parse command line options.
     *
     * We don't use boost::program_options for argument parsing because it 
     * cannot do what we need. 
     */

    QStringList args = QApplication::arguments();
    for(int i = 1; i < args.size(); ) {
      QString argName = args[i];

      /* It must be an option. */
      if(!argName.startsWith("-"))
        throw std::logic_error("Unrecognized option \"" + argName.toStdString() + "\"");

      /* Check for options without arguments. */
      if(argName == "--help") {
        std::cout << "htmlconv - html converter, version 0.1." << std::endl;
        std::cout << std::endl;
        std::cout << "USAGE:" << std::endl;
        std::cout << "  htmlconv [options]" << std::endl;
        std::cout << std::endl;
        std::cout << "General options:" << std::endl;
        std::cout << "      --help                  Produce help message." << std::endl;
        std::cout << "  -o, --output                Output file name." << std::endl;
        std::cout << "  -a, --antialiasing          Perform antialiasing." << std::endl;
        std::cout << std::endl;
        std::cout << "Image options (applicable to image output only):" << std::endl;
        std::cout << "  -w, --width                 Image width, in pixels." << std::endl;
        std::cout << "  -h, --height                Image height, in pixels. Will be determined" << std::endl;
        std::cout << "                              automatically if not set." << std::endl;
        std::cout << "  -s, --scale                 Scaling coefficient. Default is 1.0." << std::endl;
        std::cout << std::endl;
        std::cout << "Input and pdf options (can be used multiple times):" << std::endl;
        std::cout << "  -i, --input                 Input file URL." << std::endl;
        std::cout << "  -p, --ppi                   Number of html pixels per inch of pdf paper." << std::endl;
        std::cout << "  -r, --orientation           Paper orientation, must be landscape or portrait." << std::endl;
        return 1;
      } else if(argName == "-a" || argName == "--antialising") {
        antialising = true;
        i++;
        continue;
      }

      /* All other options must have an argument. */
      if(i + 1 >= args.size())
        throw std::logic_error("No value is provided for option \"" + argName.toStdString() + "\"");

      /* Extract option argument. */
      QString argValue = args[i + 1];
      i += 2;

      /* Parse option & argument value. */
      if(argName == "-o" || argName == "--output") {
        if(!outputFileName.isNull())
          throw std::logic_error("Only one output file must be provided");
        outputFileName = argValue;
      } else if(argName == "-w" || argName == "--width") {
        bool imageWidthOk;
        imageWidth = argValue.toInt(&imageWidthOk);
        if(!imageWidthOk || imageWidth <= 0)
          throw new std::logic_error("Invalid value for option " + argName.toStdString() + ": \"" + argValue.toStdString() + "\"");
      } else if(argName == "-h" || argName == "--height") {
        bool imageHeightOk;
        imageHeight = argValue.toInt(&imageHeightOk);
        if(!imageHeightOk || imageHeight <= 0)
          throw new std::logic_error("Invalid value for option " + argName.toStdString() + ": \"" + argValue.toStdString() + "\"");
      } else if(argName == "-s" || argName == "--scale") {
        bool scaleOk;
        scale = argValue.toDouble(&scaleOk);
        if(!scaleOk || scale <= 0.0)
          throw new std::logic_error("Invalid value for option " + argName.toStdString() + ": \"" + argValue.toStdString() + "\"");
      } else if(argName == "-i" || argName == "--input") {
        QUrl url(argValue);
        if(!url.isValid())
          throw new std::logic_error("Invalid url value for option " + argName.toStdString() + ": \"" + argValue.toStdString() + "\"");
        instructions.push_back(Instruction(INPUT, url));
      } else if(argName == "-p" || argName == "--ppi") {
        bool ppiOk;
        int ppi = argValue.toInt(&ppiOk);
        if(!ppiOk || ppi <= 0)
          throw std::logic_error("Invalid value for option " + argName.toStdString() + ": \"" + argValue.toStdString() + "\"");
        instructions.push_back(Instruction(PPI, ppi));
      } else if(argName == "-r" || argName == "--orientation") {
        QPrinter::Orientation orientation;
        if(argValue == "portrait")
          orientation = QPrinter::Portrait;
        else if(argValue == "landscape")
          orientation = QPrinter::Landscape;
        else
          throw std::logic_error("Invalid value for option " + argName.toStdString() + ": \"" + argValue.toStdString() + "\"");
        instructions.push_back(Instruction(ORIENTATION, orientation));
      } else {
        throw std::logic_error("Unrecognized option \"" + argName.toStdString() + "\"");
      }
    }

    /* 
     * Check that provided options are meaningful and prepare 
     * everything for rendering. 
     */

    if(outputFileName.isNull())
      throw std::logic_error("No output file provided");

    QScopedPointer<QPrinter> printer;
    QScopedPointer<QPainter> painter;
    QScopedPointer<QImage> image;
    bool printingPdf;

    if(outputFileName.endsWith(".pdf")) {
      if(imageWidth != 0)
        throw std::logic_error("Width can be set for image output only");
      if(imageHeight != 0)
        throw std::logic_error("Height can be set for image output only");
      if(scale != 0.0)
        throw std::logic_error("Scale can be set for image output only");

      printer.reset(new QPrinter());
      printer->setOutputFileName(outputFileName);
      printer->setOutputFormat(QPrinter::PdfFormat);
      printer->setPaperSize(QPrinter::A4);
      printer->setOrientation(QPrinter::Portrait);
      printer->setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);

      painter.reset(new QPainter(printer.data()));

      printingPdf = true;
    } else {
      if(imageWidth == 0)
        throw std::logic_error("Width must be set for image output (-w and --width options)");
      if(scale == 0.0)
        scale = 1.0;

      /* We cannot create QPainter & QImage here as image height is not yet known. */

      printingPdf = false;
    }

    /* 
     * Render! 
     */

    /* Are we on the first page? */
    bool onFirstPage = true; 

    /* Orientation of the current page. */
    QPrinter::Orientation orientation = QPrinter::Portrait;

    /* Pixels-per-inch of the current page. */
    int ppi = 254;

    foreach(const Instruction &instr, instructions) {
      switch(instr.first) {
      case PPI:
        ppi = instr.second.toInt();
        break;
      case ORIENTATION:
        orientation = static_cast<QPrinter::Orientation>(instr.second.toInt());
        break;
      case INPUT: {
        /* Determine width in pixels of the HTML rendering canvas. */
        int htmlWidth;
        if(printingPdf) {
          /* 8.3 and 11.7 are width and height of A4 paper in inches. */
          htmlWidth = static_cast<int>(ppi * (orientation == QPrinter::Portrait ? 8.3 : 11.7));
        } else {
          htmlWidth = static_cast<int>(imageWidth / scale);
        }

        /* Download page. */
        htmlconv::HtmlConv conv(instr.second.toUrl(), htmlWidth);
        QEventLoop loop;
        QObject::connect(&conv, SIGNAL(loadFinished()), &loop, SLOT(quit()));
        loop.exec();

        /* Prepare for rendering. */
        if(printingPdf) {
          if(onFirstPage) {
            onFirstPage = false;
          } else {
            printer->newPage();
          }

          painter->resetTransform();
          if(orientation == QPrinter::Landscape) { /* Rotate if needed. */
            painter->translate(0, 11.7 * printer->resolution());
            painter->setTransform(QTransform(0, -1, 1, 0, 0, 0), true);
          }

          double scale = static_cast<double>(printer->resolution()) / ppi;
          painter->scale(scale, scale);
        } else {
          /* Here we finally know the image height, so we can allocate it and
           * create painter. */
          image.reset(new QImage(QSize(imageWidth, imageHeight == 0 ? conv.contentsSize().height() : imageHeight), QImage::Format_ARGB32));
          painter.reset(new QPainter(image.data()));

          painter->resetTransform();
          painter->scale(scale, scale);
        }

        if(antialising)
          painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);

        /* Render! */
        conv.render(painter.data());

        break;
      }
      default:
        unreachable();
      }
    }

    painter->end();

    if(!printingPdf)
      if(!image->save(outputFileName))
        throw std::logic_error("Could not write output to file \"" + outputFileName.toStdString() + "\"");
  } catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch(...) {
    std::cerr << "error: Exception of unknown type." << std::endl;
    return 1;
  }

  return 0;
}