#include "utils/misc/variant.h"
#include "uxthread/scv/scv.h"

using namespace scv;

class PDProcessing {
  public:
    static void loadDatabase();
    static void processing(Matrix &inputImage, Variant &message);
};
