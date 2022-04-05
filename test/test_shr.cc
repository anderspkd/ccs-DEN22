#include <catch2/catch.hpp>

#include "frn/shr.h"

using namespace frn;

TEST_CASE("Add and Subtract") {
  int m = 10;
  int d = (m - 1) / 3;
  frn::lib::primitives::PRG prg;
  auto repl = CreateReplicator(m);
  Field x(10), y(20);
  Field z = x + y;
  Field w = x - y;
  std::vector<Shr> sharesx = repl.Share(x, prg);
  std::vector<Shr> sharesy = repl.Share(y, prg);
  std::vector<Shr> sharesz, sharesw;

  // Share manipulators
  std::vector<ShrManipulator> manipulators;
  for (int i = 0; i < m; i++) {
    manipulators.emplace_back(ShrManipulator(i, d, m));
  }

  // Manipulators add and subtract
  for (int i = 0; i < m; i++) {
    ShrManipulator manipulator = manipulators[i];
    sharesz.emplace_back(manipulator.Add(sharesx[i], sharesy[i]));
    sharesw.emplace_back(manipulator.Subtract(sharesx[i], sharesy[i]));
  }

  // Reconstruct result and check
  Field sum = repl.Reconstruct(sharesz);
  REQUIRE(sum == z);
  Field diff = repl.Reconstruct(sharesw);
  REQUIRE(diff == w);
}

TEST_CASE("Add and Subtract by constant") {
  int m = 10;
  int d = (m - 1) / 3;
  frn::lib::primitives::PRG prg;
  auto repl = CreateReplicator(m);
  Field x(10), y(20);
  Field z = x + y;
  Field u = x - y;
  Field v = y - x;

  std::vector<Shr> sharesx = repl.Share(x, prg);
  std::vector<Shr> sharesz, sharesu, sharesv;

  // Share manipulators
  std::vector<ShrManipulator> manipulators;
  for (int i = 0; i < m; i++) {
    manipulators.emplace_back(ShrManipulator(i, d, m));
  }

  // Manipulators add and subtract
  for (int i = 0; i < m; i++) {
    ShrManipulator manipulator = manipulators[i];
    sharesz.emplace_back(manipulator.AddConstant(sharesx[i], y));
    sharesu.emplace_back(manipulator.SubtractConstant(sharesx[i], y));
    sharesv.emplace_back(manipulator.SubtractConstant(y, sharesx[i]));
  }

  // Reconstruct result and check
  Field rec_z = repl.Reconstruct(sharesz);
  REQUIRE(rec_z == z);
  Field rec_u = repl.Reconstruct(sharesu);
  REQUIRE(rec_u == u);
  Field rec_v = repl.Reconstruct(sharesv);
  REQUIRE(rec_v == v);
}

TEST_CASE("Multiply by constant") {
  int m = 10;
  int d = (m - 1) / 3;
  frn::lib::primitives::PRG prg;
  auto repl = CreateReplicator(m);
  Field x(10), c(20);
  Field z = c * x;

  std::vector<Shr> sharesx = repl.Share(x, prg);
  std::vector<Shr> sharesz1, sharesz2;

  // Share manipulators
  std::vector<ShrManipulator> manipulators;
  for (int i = 0; i < m; i++) {
    manipulators.emplace_back(ShrManipulator(i, d, m));
  }

  // Manipulators add and subtract
  for (int i = 0; i < m; i++) {
    ShrManipulator manipulator = manipulators[i];
    sharesz1.emplace_back(manipulator.MultiplyConstant(sharesx[i], c));
    sharesz2.emplace_back(manipulator.MultiplyConstant(c, sharesx[i]));
  }

  // Reconstruct result and check
  Field rec_z1 = repl.Reconstruct(sharesz1);
  REQUIRE(rec_z1 == z);
  Field rec_z2 = repl.Reconstruct(sharesz1);
  REQUIRE(rec_z2 == z);
}

TEST_CASE("Multiplication indexes") {
  auto repl = CreateReplicator(4);
  REQUIRE(repl.AdditiveShareSize() == 4);
  std::vector<int> combination;
  // Sets of size n-d: 012 013 023 123
  // And for each party:
  // Party 0: 012 013 023
  // Party 1: 012 013 123
  // Party 2: 012 023 123
  // Party 3: 013 023 123

  // Sets of size n-2d for each party:
  // Party 0: 01 02 03
  // Party 1: 01 12 13
  // Party 2: 02 12 23
  // Party 3: 03 13 23

  auto manipulator0 = ShrManipulator(0, 1, 4);
  auto manipulator1 = ShrManipulator(1, 1, 4);
  auto manipulator2 = ShrManipulator(2, 1, 4);
  auto manipulator3 = ShrManipulator(3, 1, 4);
  int idx0, idx1, idx2, idx3;

  // Intersection of set 012 with set 013 is 01, which is set
  // #0 for P0 and P1, and doesn't appear for P2 and P3
  idx0 = manipulator0.ComputeIndexForDoubleMultiplication(0, 1);
  idx1 = manipulator1.ComputeIndexForDoubleMultiplication(0, 1);
  REQUIRE(idx0 == 0);
  REQUIRE(idx1 == 0);

  // Intersection of set 012 with itself is 012, which is set
  // #0 for P0, P1 and P2, but P2 is not within the first n-2d parties
  idx0 = manipulator0.ComputeIndexForDoubleMultiplication(0, 0);
  idx1 = manipulator1.ComputeIndexForDoubleMultiplication(0, 0);
  idx2 = manipulator2.ComputeIndexForDoubleMultiplication(0, 0);
  REQUIRE(idx0 == 0);
  REQUIRE(idx1 == 0);
  REQUIRE(idx2 == -1);

  // Intersection of set 012 with set 023 is 02, which is set
  // #1 for P0, #0 and P2, and doesn't appear for P1 and P3
  idx0 = manipulator0.ComputeIndexForDoubleMultiplication(0, 2);
  idx2 = manipulator2.ComputeIndexForDoubleMultiplication(0, 1);
  REQUIRE(idx0 == 1);
  REQUIRE(idx2 == 0);

  // Intersection of set 013 with set 123 is 13, which is set
  // #2 for P1, #1 and P3, and doesn't appear for P0 and P2
  idx1 = manipulator1.ComputeIndexForDoubleMultiplication(1, 2);
  idx3 = manipulator3.ComputeIndexForDoubleMultiplication(0, 2);
  REQUIRE(idx1 == 2);
  REQUIRE(idx3 == 1);
}

TEST_CASE("Local multiplication to double threshold") {
  int m = 8;
  int d = (m - 1) / 3;
  frn::lib::primitives::PRG prg;
  auto repl = CreateReplicator(m);
  auto repl2 = frn::lib::secret_sharing::Replicator<Field>(m, 2 * d);

  Field x(10), y(20);
  Field z = x * y;
  std::vector<Shr> sharesx = repl.Share(x, prg);
  std::vector<Shr> sharesy = repl.Share(y, prg);
  std::vector<ShrD> sharesz;

  // Share manipulators
  std::vector<ShrManipulator> manipulators;
  for (int i = 0; i < m; i++) {
    manipulators.emplace_back(ShrManipulator(i, d, m));
  }

  // Manipulators multiply locally
  for (int i = 0; i < m; i++) {
    ShrManipulator manipulator = manipulators[i];
    sharesz.emplace_back(
        manipulator.MultiplyToDoubleDegree(sharesx[i], sharesy[i]));
  }

  // Reconstruct result and check
  Field prod = repl2.Reconstruct(sharesz);
  REQUIRE(prod == z);
}

TEST_CASE("Local multiplication to additive sharing") {
  int m = 8;
  int d = (m - 1) / 3;
  frn::lib::primitives::PRG prg;
  auto repl = CreateReplicator(m);
  auto repl2 = frn::lib::secret_sharing::Replicator<Field>(m, 2 * d);

  Field x(10), y(20);
  Field z = x * y;
  std::vector<Shr> sharesx = repl.Share(x, prg);
  std::vector<Shr> sharesy = repl.Share(y, prg);
  std::vector<Field> addz;

  // Share manipulators
  std::vector<ShrManipulator> manipulators;
  for (int i = 0; i < m; i++) {
    manipulators.emplace_back(ShrManipulator(i, d, m));
  }

  // Manipulators multiply locally
  for (int i = 0; i < m; i++) {
    ShrManipulator manipulator = manipulators[i];
    addz.emplace_back(manipulator.MultiplyToAdditive(sharesx[i], sharesy[i]));
  }

  // Reconstruct result and check
  Field prod(0);
  for (int i = 0; i < 2 * d + 1; ++i) prod += addz[i];
  REQUIRE(prod == z);
}
