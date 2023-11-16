//
// Created by nawat on 17/11/2566.
//

#include <algorithm>
#include <iostream>

#include "parlay/io.h"
#include "parlay/parallel.h"
#include "parlay/primitives.h"
// #include "utils/types.h"
#include "utils/euclidian_point.h"
#include "utils/mips_point.h"
#include "utils/point_range.h"

using pid = std::pair<int, float>;

int main(int argc, char *argv[]) {
  commandLine P(
	  argc,
	  argv,
	  "[-base_path <b>] [-data_type <d>] [-dist_func <distance function>]");

  char *bFile = P.getOptionValue("-base_path");
  char *vectype = P.getOptionValue("-data_type");
  char *dfc = P.getOptionValue("-dist_func");
  std::string df = std::string(dfc);

  std::string tp = std::string(vectype);
  if ((tp != "uint8") && (tp != "int8") && (tp != "float")) {
	std::cout << "Error: data type not specified correctly, specify int8, "
				 "uint8, or float"
			  << std::endl;
	abort();
  }

  std::cout << "Printing vectors" << std::endl;

  parlay::sequence<parlay::sequence<pid>> answers;
  if (tp == "float") {
	std::cout << "Detected float coordinates" << std::endl;
	if (df == "Euclidian") {
	  std::cout << "Euclidian distance" << std::endl;
	  PointRange<float, Euclidian_Point<float>> B =
		  PointRange<float, Euclidian_Point<float>>(bFile);
	  B.print();

	  std::cout << "Slice" << std::endl;
	  auto new_base = B.get_slice(5);
	  new_base.print();
	} else if (df == "mips") {
	  PointRange<float, Mips_Point<float >> B =
		  PointRange<float, Mips_Point<float >>(bFile);
	  B.print();
	}
  } else if (tp == "uint8") {
	std::cout << "Detected uint8 coordinates" << std::endl;
	if (df == "Euclidian") {
	  PointRange<uint8_t, Euclidian_Point<uint8_t>> B =
		  PointRange<uint8_t, Euclidian_Point<uint8_t>>(bFile);
	  B.print();
	} else if (df == "mips") {
	  PointRange<uint8_t, Mips_Point<uint8_t >> B =
		  PointRange<uint8_t, Mips_Point<uint8_t >>(bFile);
	  B.print();
	}
  } else if (tp == "int8") {
	std::cout << "Detected int8 coordinates" << std::endl;
	if (df == "Euclidian") {
	  PointRange<int8_t, Euclidian_Point<int8_t >> B =
		  PointRange<int8_t, Euclidian_Point<int8_t >>(bFile);
	  B.print();
	} else if (df == "mips") {
	  PointRange<int8_t, Mips_Point<int8_t >> B =
		  PointRange<int8_t, Mips_Point<int8_t >>(bFile);
	  B.print();
	}
  }

  return 0;
}
