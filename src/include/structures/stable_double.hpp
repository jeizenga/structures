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


//  stable_double.hpp
//
// Contains an implementation of an underflow- and overflow-resistant
// alternative to floating point numbers
//

#ifndef structures_stable_double_hpp
#define structures_stable_double_hpp

#include <ostream>
#include <cmath>
#include <limits>

namespace structures {

using namespace std;

/*
 * A class that supports basic floating point arithmetic operations on internal values stored
 * in log-transformed space to reduce risk of overflow and underflow at the cost of precision
 * in some ranges of values
 */
class StableDouble {
public:
    
    /// Defaults to 0
    StableDouble();
    /// Convert from a double
    StableDouble(double x);
    /// Construct from log-transformed absolute value and a sign
    StableDouble(double log_abs_x, bool positive);
    
    /// Convert to double
    inline double to_double() const;
    
    /// Unary operators
    
    inline StableDouble operator-() const;
    inline StableDouble inverse() const;
    
    /// Binary operators with StableDoubles
    
    inline StableDouble operator*(const StableDouble& other) const;
    inline StableDouble operator/(const StableDouble& other) const;
    inline StableDouble operator+(const StableDouble& other) const;
    inline StableDouble operator-(const StableDouble& other) const;
    
    /// Binary operators with standard doubles
    
    inline StableDouble operator*(const double other) const;
    inline StableDouble operator/(const double other) const;
    inline StableDouble operator+(const double other) const;
    inline StableDouble operator-(const double other) const;
    
    /// Assignment operators with StableDoubles
    
    inline StableDouble& operator*=(const StableDouble& other);
    inline StableDouble& operator/=(const StableDouble& other);
    inline StableDouble& operator+=(const StableDouble& other);
    inline StableDouble& operator-=(const StableDouble& other);
    
    /// Assignment operators with standard doubles
    
    inline StableDouble& operator*=(const double other);
    inline StableDouble& operator/=(const double other);
    inline StableDouble& operator+=(const double other);
    inline StableDouble& operator-=(const double other);
    
    // Comparison operators with StableDoubles
    
    inline bool operator<(const StableDouble& other) const;
    inline bool operator>(const StableDouble& other) const;
    inline bool operator<=(const StableDouble& other) const;
    inline bool operator>=(const StableDouble& other) const;
    
    // Comparison operators with standard doubles
    
    inline bool operator<(const double other) const;
    inline bool operator>(const double other) const;
    inline bool operator<=(const double other) const;
    inline bool operator>=(const double other) const;
    
    // Equality operators with StableDoubles
    
    inline bool operator==(const StableDouble& other) const;
    inline bool operator!=(const StableDouble& other) const;
    
    // Equality operators with standard doubles
    
    inline bool operator==(const double other) const;
    inline bool operator!=(const double other) const;
    
    friend ostream& operator<<(ostream& out, const StableDouble& val);
    
private:
    
    // Logarithm of the absolute value
    double log_abs_x;
    // Sign
    bool positive;
    
    // Returns the log of the sum of two log-transformed values without taking them
    // out of log space.
    inline double add_log(const double log_x, const double log_y) const;
    
    // Returns the log of the difference of two log-transformed values without taking
    // them out of log space.
    inline double subtract_log(const double log_x, const double log_y) const;
};

ostream& operator<<(ostream& out, const StableDouble& val);




inline double StableDouble::add_log(const double log_x, const double log_y) const {
    return log_x > log_y ? log_x + log(1.0 + exp(log_y - log_x)) : log_y + log(1.0 + exp(log_x - log_y));
}

inline double StableDouble::subtract_log(const double log_x, const double log_y) const {
    return log_x + log(1.0 - exp(log_y - log_x));
}

inline double StableDouble::to_double() const {
    return positive ? exp(log_abs_x) : -exp(log_abs_x);
}

inline StableDouble StableDouble::inverse() const {
    return StableDouble(-log_abs_x, positive);
}

inline StableDouble StableDouble::operator-() const {
    return StableDouble(log_abs_x, !positive);
}

inline StableDouble StableDouble::operator*(const StableDouble& other) const {
    return StableDouble(log_abs_x + other.log_abs_x, positive == other.positive);
}

inline StableDouble StableDouble::operator/(const StableDouble& other) const {
    return StableDouble(log_abs_x - other.log_abs_x, positive == other.positive);
}

inline StableDouble StableDouble::operator+(const StableDouble& other) const {
    if (positive == other.positive) {
        return StableDouble(add_log(log_abs_x, other.log_abs_x), positive);
    }
    else if (log_abs_x == other.log_abs_x) {
        return StableDouble();
    }
    else if (log_abs_x > other.log_abs_x) {
        return StableDouble(subtract_log(log_abs_x, other.log_abs_x), positive);
    }
    else {
        return StableDouble(subtract_log(other.log_abs_x, log_abs_x), other.positive);
    }
}

inline StableDouble StableDouble::operator-(const StableDouble& other) const {
    return *this + (-other);
}

inline StableDouble StableDouble::operator*(const double other) const {
    return *this * StableDouble(other);
}

inline StableDouble StableDouble::operator/(const double other) const {
    return *this / StableDouble(other);
}

inline StableDouble StableDouble::operator+(const double other) const {
    return *this + StableDouble(other);
}

inline StableDouble StableDouble::operator-(const double other) const {
    return *this - StableDouble(other);
}

inline StableDouble& StableDouble::operator*=(const StableDouble& other) {
    *this = *this * other;
    return *this;
}

inline StableDouble& StableDouble::operator/=(const StableDouble& other) {
    *this = *this / other;
    return *this;
}

inline StableDouble& StableDouble::operator+=(const StableDouble& other) {
    *this = *this + other;
    return *this;
}

inline StableDouble& StableDouble::operator-=(const StableDouble& other) {
    *this = *this - other;
    return *this;
}

inline StableDouble& StableDouble::operator*=(const double other) {
    *this = *this * other;
    return *this;
}

inline StableDouble& StableDouble::operator/=(const double other) {
    *this = *this / other;
    return *this;
}

inline StableDouble& StableDouble::operator+=(const double other) {
    *this = *this + other;
    return *this;
}

inline StableDouble& StableDouble::operator-=(const double other) {
    *this = *this - other;
    return *this;
}

inline bool StableDouble::operator<(const StableDouble& other) const {
    if (positive != other.positive) {
        // make sure they're not both 0 with the sign arbitrarily set
        return other.positive && (log_abs_x != numeric_limits<double>::lowest() ||
                                  other.log_abs_x != numeric_limits<double>::lowest());
    }
    else {
        return positive ? (log_abs_x < other.log_abs_x) : (log_abs_x > other.log_abs_x) ;
    }
}

inline bool StableDouble::operator>(const StableDouble& other) const {
    return other < *this;
}

inline bool StableDouble::operator<=(const StableDouble& other) const {
    return !(other < *this);
}

inline bool StableDouble::operator>=(const StableDouble& other) const {
    return !(*this < other);
}

inline bool StableDouble::operator<(const double other) const {
    return *this < StableDouble(other);
}

inline bool StableDouble::operator>(const double other) const {
    return *this > StableDouble(other);
}

inline bool StableDouble::operator<=(const double other) const {
    return *this <= StableDouble(other);
}

inline bool StableDouble::operator>=(const double other) const {
    return *this >= StableDouble(other);
}

inline bool StableDouble::operator==(const StableDouble& other) const {
    return ((log_abs_x == other.log_abs_x) &&
            (log_abs_x == numeric_limits<double>::lowest() || positive == other.positive));
}

inline bool StableDouble::operator!=(const StableDouble& other) const {
    return !(*this == other);
}

inline bool StableDouble::operator==(const double other) const {
    return *this == StableDouble(other);
}

inline bool StableDouble::operator!=(const double other) const {
    return *this != StableDouble(other);
}

}

#endif /* structures_stable_double_hpp */
