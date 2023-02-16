#include "APU.h"
#include <stdlib.h>

APU* APU_Create(void) {
    APU* apu = malloc(sizeof(APU));
    return apu;
}

void APU_Destroy(APU* apu) {
    free(apu);
}

double APU_GetOutputSample(APU* apu) {
    return apu->pulse1->sample;
}
