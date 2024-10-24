#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>

#include "APCommon.h"


double linearToExponential(double linearValue, double minValue, double maxValue)
{
    linearValue = std::clamp(linearValue, minValue, maxValue);
    double normalized = (linearValue - minValue) / (maxValue - minValue);
    double exponentialValue = std::pow(normalized, 2.0);
    double result = minValue + exponentialValue * (maxValue - minValue);
    return result;
}

float gainToDecibels(float gain) {
    if (gain <= 0.0)
        return -1000;
    
    if (gain > 1000) gain = 1000;
    
    return 20.0 * log10f(gain);
}

float decibelsToGain(float decibels) {
    if (decibels <= -1000)
        return 0.0;
    
    if (decibels > 1000) decibels = 1000;
    
    return powf(10.0, decibels / 20.0);
}

std::string floatToStringWithTwoDecimalPlaces(float value) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}


ParameterQuery queryParameter(ParameterNames paramName, const std::string& parameterStringName) {

    static const std::unordered_map<ParameterNames, ParameterQuery> paramNameMap = {
        {ParameterNames::inGain,       { "inGain",       "Input Gain",      ParameterNames::inGain  }},
        {ParameterNames::outGain,      { "outGain",      "Output Gain",     ParameterNames::outGain }},
        {ParameterNames::selection,    { "selection",    "Saturation Type", ParameterNames::selection }},
    };
    
    if (paramName != ParameterNames::END) {
        auto it = paramNameMap.find(paramName);
        if (it != paramNameMap.end()) return it->second;
    }

    static const std::unordered_map<std::string, ParameterNames> nameToEnumMap = {
        {"inGain",        ParameterNames::inGain},
        {"outGain",       ParameterNames::outGain},
        {"selection",     ParameterNames::selection},
    };
    
    auto strIt = nameToEnumMap.find(parameterStringName);
    if (strIt != nameToEnumMap.end()) return queryParameter(strIt->second);

    throw std::invalid_argument("Both enum and string queries failed for parameter for queryParameter");
}
