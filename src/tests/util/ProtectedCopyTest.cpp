#include "ProtectedCopy.hpp"

using smsc::test::util::ProtectedCopy;

void test(ProtectedCopy c) {
}

class Copy : public ProtectedCopy {
};

int main(int argc, char** argv) {
  ProtectedCopy c1;
  //ProtectedCopy c2 = c1;
  //test(c1);

  Copy cc1;
  //Copy cc2 = cc1;
  //test(cc1);
}

