#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Point_set_3.h>

#include <fstream>
#include <limits>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::FT FT;
typedef Kernel::Point_3 Point;
typedef Kernel::Vector_3 Vector;

typedef CGAL::Point_set_3<Point> Point_set;

int main (int argc, char** argv)
{
  const char* fname (argc > 1 ? argv[1] : "data/oni.xyz");

  Point_set point_set;

  // Reading input in XYZ format
  if (!CGAL::read_point_set(fname, point_set))
    {
      std::cerr << "Can't read input file " << std::endl;
      return EXIT_FAILURE;
    }

  if (point_set.has_normal_map())
    {
      // Normalization + inversion of normal vectors
      for (Point_set::iterator it = point_set.begin(); it != point_set.end(); ++ it)
        {
          Vector n = point_set.normal(*it);
          n = - n / std::sqrt (n * n);
          point_set.normal(*it) = n;
        }
    }

  // Writing result in OFF format
  std::ofstream out("normalized_normals.off");
  out.precision(17);
  if (!out || !CGAL::write_OFF (out, point_set))
    {
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
