#pragma once

#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/GMockInclude.h"
#include "UnitTests/UnitTests.h"


#define DAVA_TARC_TESTCLASS(classname) \
    DAVA_TESTCLASS_CUSTOM_BASE_AND_FACTORY(classname, DAVA::TArcTestClass, DAVA::TArcTestClassHolderFactory)

#define BEGIN_TESTED_MODULES() \
    void CreateTestedModules() override {

#define DECLARE_TESTED_MODULE(moduleTypeName) \
        core->CreateModule<moduleTypeName>();

#define END_TESTED_MODULES() \
    }
