#include "utils/misc/variant.h"
#include "uxthread/scv/scv.h"

using namespace scv;

class MDProcessing {
  public:
    static void backGroundClear();
    static void processing(Variant _defaultROI, Variant _MDSelectedROI, Matrix &inputImage, Variant &message);
};
