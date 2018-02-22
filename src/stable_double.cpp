// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.


#include "structures/stable_double.hpp"


StableDouble::StableDouble() : log_abs_x(numeric_limits<double>::lowest()), positive(true) {
    
}

StableDouble::StableDouble(double x) {
    if (x == 0.0) {
        log_abs_x = numeric_limits<double>::lowest();
        positive = true;
    }
    else if (x < 0) {
        log_abs_x = log(-x);
        positive = false;
    }
    else {
        log_abs_x = log(x);
        positive = true;
    }
}

StableDouble::StableDouble(double log_abs_x, bool positive) : log_abs_x(log_abs_x), positive(positive)  {
    
}

ostream& operator<<(ostream& out, const StableDouble& val) {
    return out << (val.positive ? "exp(" : "-exp(") << val.log_abs_x << ")";
}
