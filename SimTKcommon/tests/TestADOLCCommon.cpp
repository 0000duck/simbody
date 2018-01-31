/* -------------------------------------------------------------------------- *
 *                       Simbody(tm): SimTKcommon                             *
 * -------------------------------------------------------------------------- *
 * This is part of the SimTK biosimulation toolkit originating from           *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org/home/simbody.  *
 *                                                                            *
 * Portions copyright (c) 2010-17 Stanford University and the Authors.        *
 * Authors: Antoine Falisse                                                   *
 * Contributors: Michael Sherman, Chris Dembia                                *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "SimTKcommon.h"
#include "SimTKcommon/Testing.h"
#include <adolc/adolc.h> // for jacobian() ADOL-C driver

#include <iostream>
using std::cout;
using std::endl;

using namespace SimTK;

// Test derivative of simple function with ADOLC without Simbody; just to make
// sure that ADOLC is included properly
void testDerivativeADOLC() {
    double xp[1];
    xp[0] = -2.3;

    trace_on(1);
    adouble x;
    adouble y;
    x <<= xp[0];
    y = 3*pow(x,3)+cos(x)+1;
    double y0;
    y >>= y0;
    trace_off();

    double** J;
    J = myalloc(1,1);
    jacobian(1, 1, 1, xp, J);
    SimTK_TEST(J[0][0] == 9*pow(x,2)-sin(x));
    myfree(J);
}

// Various unit tests verifying that NTraits<adouble> works properly
void testNTraitsADOLC() {
    // Widest
    constexpr bool wfad =
        std::is_same<SimTK::Widest<float, adouble>::Type, adouble>::value;
    constexpr bool wadf =
        std::is_same<SimTK::Widest<adouble, float>::Type, adouble>::value;
    constexpr bool wdad =
        std::is_same<SimTK::Widest<double, adouble>::Type, adouble>::value;
    constexpr bool wadd =
        std::is_same<SimTK::Widest<adouble, double>::Type, adouble>::value;
    constexpr bool wadad =
        std::is_same<SimTK::Widest<adouble, adouble>::Type, adouble>::value;
    SimTK_TEST(wfad);
    SimTK_TEST(wadf);
    SimTK_TEST(wdad);
    SimTK_TEST(wadd);
    SimTK_TEST(wadad);
    // Narrowest
    bool nfad =
        std::is_same<SimTK::Narrowest<float, adouble>::Type, adouble>::value;
    bool nadf =
        std::is_same<SimTK::Narrowest<adouble, float>::Type, adouble>::value;
    bool ndad =
        std::is_same<SimTK::Narrowest<double, adouble>::Type, adouble>::value;
    bool nadd =
        std::is_same<SimTK::Narrowest<adouble, double>::Type, adouble>::value;
    bool nadad =
        std::is_same<SimTK::Narrowest<adouble, adouble>::Type, adouble>::value;
    SimTK_TEST(nfad);
    SimTK_TEST(nadf);
    SimTK_TEST(ndad);
    SimTK_TEST(nadd);
    SimTK_TEST(nadad);
    // isNaN, isFinite, isInf
    adouble xad = -9.45;
    adouble xNaN = SimTK::NaN;
    adouble xInf = SimTK::Infinity;
    SimTK_TEST(isNaN(xNaN));
    SimTK_TEST(!isNaN(xad));
    SimTK_TEST(isFinite(xad));
    SimTK_TEST(!isFinite(xNaN));
    SimTK_TEST(!isFinite(xInf));
    SimTK_TEST(isInf(xInf));
    SimTK_TEST(!isInf(xad));
    // isNumericallyEqual
    double xd = -9.45;
    float xf = (float)-9.45;
    adouble yad = -9;
    int yi = -9;
    std::complex<float> cf(xf,0.);
    std::complex<double> cd(xd,0.);
    SimTK::conjugate<float> cjf(xf,0);
    SimTK::conjugate<double> cjd(xd,0.);
    SimTK_TEST(isNumericallyEqual(xad,xd));
    SimTK_TEST(isNumericallyEqual(xd,xad));
    SimTK_TEST(isNumericallyEqual(xad,xad));
    SimTK_TEST(isNumericallyEqual(xad,xf));
    SimTK_TEST(isNumericallyEqual(xf,xad));
    SimTK_TEST(isNumericallyEqual(yad,yi));
    SimTK_TEST(isNumericallyEqual(yi,yad));
    SimTK_TEST(isNumericallyEqual(cd,xad));
    SimTK_TEST(isNumericallyEqual(xad,cd));
    SimTK_TEST(isNumericallyEqual(cf,xad));
    SimTK_TEST(isNumericallyEqual(xad,cf));
    SimTK_TEST(isNumericallyEqual(cjd,xad));
    SimTK_TEST(isNumericallyEqual(xad,cjd));
    SimTK_TEST(isNumericallyEqual(cjf,xad));
    SimTK_TEST(isNumericallyEqual(xad,cjf));
}

// This test should throw an exception when using value() while taping
void testExceptionTaping() {
    adouble a = 5.;
    double b = NTraits<adouble>::value(a);
    SimTK_TEST(b == 5);

    trace_on(0);
    SimTK_TEST_MUST_THROW_EXC(NTraits<adouble>::value(a),
        SimTK::Exception::ADOLCTapingNotAllowed
    );
    trace_off();
}

// Various unit tests verifying that negator<adouble> works properly
void testNegator() {
    // Test evaluation of simple function and its derivative
    double xp[1];
    xp[0] = 2;
    const short int TraceTag = 2;
    trace_on(TraceTag);
    adouble x, y; // Declare active variables
    x <<= xp[0]; // Select independent variable
    auto result = NTraits<adouble>::pow(x,3);
    // Negate the result by reinterpretation rather than computation
    y = reinterpret_cast<const negator<adouble>&>(result);
    double y0;
    y >>= y0; // Select dependent variable
    trace_off();
    // Function evaluation. function(arg1,arg2,arg3,arg4,arg5) evaluates the
    // desired function from the tape instead of executing the corresponding
    // source code. arg1 is the tape identification, arg2 the number of
    // dependent variables m, arg3 the number of independent variables n, arg4
    // the independent vector x, and arg4 the dependent vector y = f(x) with
    // f: R(n) -> R(m). Please see ADOL-C manual for more details.
    double f[1];
    function(TraceTag, 1, 1, xp, f);
    SimTK_TEST(f[0] == -8.);
    // Derivative evaluation. gradient(arg1,arg2,arg3,arg4) evaluates the
    // the desired function derivative. arg1 is the tape identification, arg2
    // the number of independent variables n (the number of dependent variables
    // m = 1), arg3 the independent vector x, and arg4 the resulting gradient
    // of f(x). Please see ADOL-C manual for more details.
    double g[1];
    gradient(TraceTag, 1, xp, g);
    SimTK_TEST(g[0] == -3*NTraits<adouble>::pow(x,2));
    // isNumericallyEqual
    adouble xd = 9.45;
    auto& nxd = reinterpret_cast<const negator<adouble>&>(xd);
    SimTK_TEST(isNumericallyEqual(-xd,nxd));
    // isNaN, isFinite, isInf
    adouble xad = -9.45;
    adouble xNaN = SimTK::NaN;
    adouble xInf = SimTK::Infinity;
    auto& nxad = reinterpret_cast<const negator<adouble>&>(xad);
    auto& nxNaN = reinterpret_cast<const negator<adouble>&>(xNaN);
    auto& nxInf = reinterpret_cast<const negator<adouble>&>(xInf);
    SimTK_TEST(isNaN(nxNaN));
    SimTK_TEST(!isNaN(nxad));
    SimTK_TEST(isFinite(nxad));
    SimTK_TEST(!isFinite(nxNaN));
    SimTK_TEST(!isFinite(nxInf));
    SimTK_TEST(isInf(nxInf));
    SimTK_TEST(!isInf(nxad));
    // Ensure consistent behavior between double and adouble. The values should
    // be unchanged by conversion to the negated type and negated by
    // reinterpretation as the negated type.
    double a = 5;
    adouble ad = 5;
    SimTK_TEST(static_cast<negator<double>>(a) == a);
    SimTK_TEST(static_cast<negator<adouble>>(ad) == ad);
    SimTK_TEST(reinterpret_cast<negator<double>&>(a) == -a);
    SimTK_TEST(reinterpret_cast<negator<adouble>&>(ad) == -ad);
}

// Various unit tests verifying that cast() works properly
void testCast() {
    // cast an adouble to a double
    adouble a = 5.;
    double b = NTraits<adouble>::cast<double>(a);
    SimTK_TEST(b == a);
    // cast an adouble to a double when taping, this should throw an exception
    trace_on(3);
    SimTK_TEST_MUST_THROW_EXC(NTraits<adouble>::cast<double>(a),
        SimTK::Exception::ADOLCTapingNotAllowed
    );
    trace_off();
    // cast an adouble to an adouble when taping
    trace_on(4);
    adouble c = NTraits<adouble>::cast<adouble>(a);
    trace_off();
    SimTK_TEST(c == a);
}

int main() {
    SimTK_START_TEST("TestADOLCCommon");
        SimTK_SUBTEST(testDerivativeADOLC);
        SimTK_SUBTEST(testNTraitsADOLC);
        SimTK_SUBTEST(testExceptionTaping);
        SimTK_SUBTEST(testNegator);
        SimTK_SUBTEST(testCast);
    SimTK_END_TEST();
}