/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef oatpp_data_mapping_type_InterpretationTest_hpp
#define oatpp_data_mapping_type_InterpretationTest_hpp

#include "oatpp-test/UnitTest.hpp"

namespace oatpp { namespace data { namespace mapping { namespace  type {

class InterpretationTest : public oatpp::test::UnitTest {
public:

  InterpretationTest():UnitTest("TEST[data::mapping::type::InterpretationTest]"){}
  void onRun() override;

};

}}}}

#endif // oatpp_data_mapping_type_InterpretationTest_hpp
