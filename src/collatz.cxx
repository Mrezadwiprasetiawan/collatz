#include <qmainwindow.h>

#include <ArrowPlotWidget.hxx>
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QSurfaceFormat>
#include <arrowplot.hxx>
#include <collatz_cube.hxx>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

using U64 = uint64_t;

static void print_help(const char *prog) {
  std::cerr << "Usage:\n"
               "  "
            << prog
            << " tensor [Z] [Y] [X]      Print tensor values (default Z=Y=X=5)\n"
               "  "
            << prog
            << " path   [FROM] [TO]       Plot Collatz paths for seeds FROM..TO (default 1..5)\n"
               "  "
            << prog << " PATH [SEED]      Plot Collatz path only for this seed\n  " << prog
            << " all    [Z] [Y] [X]       Both tensor and paths, seeds 1..Z\n"
               "  "
            << prog
            << " -h | --help              Show this help\n"
               "\n"
               "Options:\n"
               "  Z, Y, X     Tensor dimensions (uint64, Z=depth, Y=row, X=col)\n"
               "  FROM, TO    Seed range inclusive (uint64, FROM >= 1)\n"
               "\n"
               "Examples:\n"
               "  "
            << prog
            << " tensor 3 4 5\n"
               "  "
            << prog
            << " path 1 20\n"
               "  "
            << prog << " all 6\n";
}

static void do_tensor(U64 Z, U64 Y, U64 X) {
  for (U64 z = 0; z < Z; ++z) {
    for (U64 y = 0; y < Y; ++y) {
      for (U64 x = 0; x < X; ++x) {
        std::cout << CollatzCube<U64, U64>::get_value_from_index(z, y, x);
        if (x + 1 < X) std::cout << '\t';
      }
      std::cout << '\n';
    }
    std::cout << '\n';
  }
}

static void do_path(U64 from, U64 to) {
  using namespace std;
  QSurfaceFormat fmt;
  fmt.setVersion(3, 3);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  fmt.setSamples(4);
  QSurfaceFormat::setDefaultFormat(fmt);

  int          dargc = 1;
  char       **dargv = {};
  QApplication app(dargc, dargv);
  QScreen     *screen       = QGuiApplication::primaryScreen();
  int          screenWidth  = screen->geometry().width();
  int          screenHeight = screen->geometry().height();
  QMainWindow  w;
  w.resize(screenWidth, screenHeight);
  auto widget = new ArrowPlotWidget(&w);
  w.setCentralWidget(widget);

  ap::Config cfg;
  cfg.nodeRadius   = 0.032f;
  cfg.sphereStacks = 16;
  cfg.sphereSlices = 24;
  cfg.shaftRadius  = 0.012f;
  cfg.shaftSlices  = 14;
  cfg.headLength   = 0.07f;
  cfg.headRadius   = 0.032f;
  cfg.headSlices   = 20;
  cfg.padding      = 0.15f;
  widget->config() = cfg;

  ap::ColourScheme col;
  col.nodeR         = 1.f;
  col.nodeG         = 1.f;
  col.nodeB         = 1.f;
  col.nodeA         = 1.f;
  col.shaftR        = 0.f;
  col.shaftG        = 0.83f;
  col.shaftB        = 0.67f;
  col.shaftA        = 1.f;
  col.headR         = 1.f;
  col.headG         = 0.45f;
  col.headB         = 0.f;
  col.headA         = 1.f;
  widget->colours() = col;

  vector<double> z, y, x;
  for (U64 n = from; n <= to; ++n)
    for (auto i : CollatzCube<U64, U64>::get_path_index_from_seed(n)) widget->plot().add(i.x, i.y, i.z);
  w.show();
  app.exec();
}

int main(int argc, const char **argv) {
  using namespace std;

  if (argc < 2 || string_view(argv[1]) == "-h" || string_view(argv[1]) == "--help") {
    print_help(argv[0]);
    return argc < 2 ? 1 : 0;
  }

  string_view mode(argv[1]);

  if (mode == "tensor") {
    U64 Z = argc > 2 ? stoull(argv[2]) : 5;
    U64 Y = argc > 3 ? stoull(argv[3]) : 5;
    U64 X = argc > 4 ? stoull(argv[4]) : 5;
    do_tensor(Z, Y, X);

  } else if (mode == "path") {
    U64 from = argc > 2 ? stoull(argv[2]) : 1;
    U64 to;
    if (argc > 3) to = stoull(argv[3]);
    else {
      do_path(from, from);
      return 0;
    }
    if (from < 1) {
      cerr << "Error: FROM must be >= 1\n";
      return 1;
    }
    if (from > to) {
      cerr << "Error: FROM must be <= TO\n";
      return 1;
    }
    do_path(from, to);

  } else if (mode == "all") {
    U64 Z = argc > 2 ? stoull(argv[2]) : 5;
    U64 Y = argc > 3 ? stoull(argv[3]) : 5;
    U64 X = argc > 4 ? stoull(argv[4]) : 5;
    do_tensor(Z, Y, X);

  } else {
    cerr << "Error: unknown mode '" << mode << "'\n\n";
    print_help(argv[0]);
    return 1;
  }

  return 0;
}
