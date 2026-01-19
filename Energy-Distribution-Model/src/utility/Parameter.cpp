#include "pch.h"
#include "Parameter.h"

#include "FileUtils.h"
#include <ImGuiUtils.h>

namespace General
{
    // general parameter 
    static GeneralParameter generalParameters;

    GeneralParameter GetParameters()
    {
        return generalParameters;
    }

    void SetParameters(const GeneralParameter& params)
    {
        generalParameters = params;
    }

    void ShowParameterControls()
    {
        generalParameters.ShowControls();
    }

}

std::string MCMC_Parameter::ToString() const
{
    return fmt::format(
        "# MCMC Parameter:\n"
        "#   number of samples           : {}\n"
        "#   burn in                     : {}\n"
        "#   lag                         : {}\n"
        "#   proposal sigma              : {:.4f}, {:.4f}, {:.3f} m\n"
        "#   seed                        : {}\n",
        numberSamples,
        burnIn,
        lag,
        proposalSigma[0],
        proposalSigma[1],
        proposalSigma[2],
        seed
    );
}

void MCMC_Parameter::FromString(std::string& str)
{
    std::string line;
    std::stringstream ss(str);

    while (std::getline(ss, line))
    {
        std::vector<std::string> tokens = FileUtils::SplitLine(line, ":");

        if (tokens.size() != 2)
        {
            continue;
        }

        std::string key = tokens.at(0);
        FileUtils::RemoveSubstrings(key, { "#" });
        FileUtils::TrimSpaces(key);

        std::string value = tokens.at(1);
        // remove units
        FileUtils::RemoveSubstrings(value, { "m" });
        FileUtils::TrimSpaces(value);

        if (key == "number of samples")
        {
            numberSamples = std::stoi(value);
        }
        else if (key == "burn in")
        {
            burnIn = std::stoi(value);
        }
        else if (key == "lag")
        {
            lag = std::stoi(value);
        }
        else if (key == "seed")
        {
            seed = std::stoi(value);
        }
        else if (key == "proposal sigma")
        {
            std::vector<std::string> tokens = FileUtils::SplitLine(value, ",");
            for (int i = 0; i < 3; ++i)
            {
                proposalSigma[i] = std::stof(tokens.at(i));
            }
        }
    }
}

bool MCMC_Parameter::ShowControls()
{
    ImGui::BeginGroup();
    ImGui::PushItemWidth(200.0f);

    static const unsigned int numThreads = std::thread::hardware_concurrency();
    if (ImGui::InputInt("chain length", &numberSamples, numThreads))
    {
        numberSamples = ((numberSamples + numThreads - 1) / numThreads) * numThreads;

    }
    ImGuiUtils::TextTooltip("number of final samples in the chain after burn-in and including the lag. Actual number of computed samples is higher.");
    ImGui::InputInt("burn in", &burnIn);
    ImGuiUtils::TextTooltip("number of initial samples that are discarded.");
    ImGui::InputInt("lag", &lag);
    ImGuiUtils::TextTooltip("number of samples that are skipped between two recorded samples.");

    ImGui::BeginDisabled(automaticProposalStd);
    ImGui::InputFloat3("##proposal sigmas", proposalSigma, "%.4f");
    ImGuiUtils::TextTooltip("standard deviations of the normal distributions used to propose new samples in x,y,z");
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Checkbox("auto set proposal sigma (x,y,z)", &automaticProposalStd);
    ImGuiUtils::TextTooltip("sets the proposal sigmas to the standard deviations of the target distribution in x,y,z");

    ImGui::BeginDisabled(changeSeed);
    ImGui::InputInt("##seed", &seed);
    ImGuiUtils::TextTooltip("seed for the random number generator");
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Checkbox("auto seed", &changeSeed);
    ImGuiUtils::TextTooltip("sets the seed to the current time");

    ImGui::SeparatorText("generation options");
    ImGui::Checkbox("async", &generateAsync);
    ImGuiUtils::TextTooltip("generates the chain using multiple threads asynchronously");
    ImGui::SameLine();
    ImGui::Checkbox("interpolate", &useInterpolation);
    ImGuiUtils::TextTooltip("uses interpolation to get values between histogram bins instead of the closest bin value.");

    ImGui::PopItemWidth();
    ImGui::EndGroup();

    return false;
}

std::string ElectronBeamParameter::ToString() const
{
    std::string result = fmt::format(
        "# Electron Beam Parameter:\n"
        "#   detuning energy             : {:.6e} eV\n"
        "#   detuning velocity           : {:.2f} m/s\n"
        "#   transverse kT               : {:.2e} eV\n"
        "#   estimated longitudinal kT   : {:.2e} eV\n"
        "#   cooling energy              : {:.6e} eV\n"
        "#   cathode radius              : {:.3e} m\n"
        "#   expansion factor            : {:.1f}\n"
        "#   electron current            : {:.2e} A\n"
        "#   cathode temperature         : {:.1f} K\n"
        "#   LLR                         : {:.1f}\n"
        "#   sigma lab energy            : {:.3f} eV\n"
        "#   extraction energy           : {:.3f} eV\n"
        "#   density file                : {}\n",
        detuningEnergy          ,
        detuningVelocity        ,
        transverse_kT           ,
        longitudinal_kT_estimate,
        coolingEnergy           ,
        cathodeRadius           ,
        expansionFactor         ,
        electronCurrent         ,
        cathodeTemperature      ,
        LLR                     ,
        sigmaLabEnergy          ,
        extractionEnergy        ,
        densityFile
    );

    if (gaussianElectronBeam)
    {
        result += "#   used gaussian electron beam ";
    }
    else if (cylindricalElectronBeam)
    {
        result += "#   used cylindrical electron beam ";
    }
    
    if (gaussianElectronBeam || cylindricalElectronBeam)
    {
        if (noElectronBeamBend)
        {
            result += "without bend\n";
        }
        else
        {
            result += "with bend\n";
        }

        result += fmt::format(
        "#   radius                      : {:.2e} m\n"
        "#   density                     : {:.2e} 1/m^3\n",
            electronBeamRadius,
            electronBeamDensity
        );
    }
    if (fixedLongitudinalTemperature)
    {
        result += "#   used fixed longitudinal temperature\n";
    }

    return result;
}

void ElectronBeamParameter::FromString(std::string& str)
{
    std::string line;
    std::stringstream ss(str);

    while (std::getline(ss, line))
    {
        std::vector<std::string> tokens = FileUtils::SplitLine(line, ":");

        std::string key = tokens.at(0);
        FileUtils::RemoveSubstrings(key, { "#" });
        FileUtils::TrimSpaces(key);

        if (tokens.size() == 1)
        {
            if (key.find("used gaussian electron beam") != std::string::npos)
            {
                gaussianElectronBeam = true;
            }
            if (key.find("used cylindrical electron beam") != std::string::npos)
            {
                cylindricalElectronBeam = true;
            }
            if (key.find("with bend") != std::string::npos)
            {
                noElectronBeamBend = false;
            }
            if (key.find("without bend") != std::string::npos)
            {
                noElectronBeamBend = true;
            }
            if (key == "used fixed longitudinal temperature")
            {
                fixedLongitudinalTemperature = true;
            }
        }
        if (tokens.size() != 2)
        {
            continue;
        }

        std::string value = tokens.at(1);
        FileUtils::TrimSpaces(value);
        if (key != "density file")
        {
            // remove units
            FileUtils::RemoveSubstrings(value, {"m/s", "eV", "K", "A" });
        }

        if (key == "detuning energy")
        {
            detuningEnergy = std::stod(value);
        }
        else if (key == "detuning velocity")
        {
            detuningVelocity = std::stod(value);
        }
        else if (key == "transverse kT")
        {
            transverse_kT = std::stod(value);
        }
        else if (key == "estimated longitudinal kT")
        {
            longitudinal_kT_estimate = std::stod(value);
        }
        else if (key == "cooling energy")
        {
            coolingEnergy = std::stod(value);
        }
        else if (key == "cathode radius")
        {
            cathodeRadius = std::stod(value);
        }
        else if (key == "expansion factor")
        {
            expansionFactor = std::stod(value);
        }
        else if (key == "electron current")
        {
            electronCurrent = std::stod(value);
        }
        else if (key == "cathode temperature")
        {
            cathodeTemperature = std::stod(value);
        }
        else if (key == "LLR")
        {
            LLR = std::stod(value);
        }
        else if (key == "sigma lab energy")
        {
            sigmaLabEnergy = std::stod(value);
        }
        else if (key == "extraction energy")
        {
            extractionEnergy = std::stod(value);
        }
        else if (key == "density file")
        {
            densityFile = value;
        }
        else if (key == "radius")
        {
            electronBeamRadius = std::stod(value);
        }
        else if (key == "density")
        {
            electronBeamDensity = std::stod(value);
        }
    }
}

bool ElectronBeamParameter::ShowControls()
{
    ImGui::BeginGroup();

    ImGui::PushItemWidth(90.0f);

    ImGui::Checkbox("use fixed longitudinal kT", &fixedLongitudinalTemperature);
    ImGui::BeginDisabled(!fixedLongitudinalTemperature);
    ImGui::InputDouble("longitudinal kT [eV]", &longitudinal_kT_estimate);
    ImGui::EndDisabled();
    ImGui::InputDouble("cooling energy [eV]", &coolingEnergy);
    ImGui::InputDouble("transverse kT [eV]", &transverse_kT);
    ImGui::BeginDisabled(fixedLongitudinalTemperature);
    ImGui::InputDouble("cathode radius [m]", &cathodeRadius);
    ImGui::InputDouble("cathode Temperature [K]", &cathodeTemperature);
    ImGui::InputDouble("extraction energy [eV]", &extractionEnergy);
    ImGuiUtils::TextTooltip("extraction energy after space charge and contact potential correction");
    ImGui::InputDouble("expansion factor", &expansionFactor);
    ImGui::InputDouble("LLR", &LLR);
    ImGui::InputDouble("sigma lab energy [eV]", &sigmaLabEnergy);
    ImGui::EndDisabled();

    ImGui::SeparatorText("Loading options");
    ImGui::Checkbox("increase bin number", &increaseHist);
    ImGuiUtils::TextTooltip("artificially increases the number of bins by an odd factor by interpolating. Increases computation time.");
    ImGui::SameLine();
    ImGui::BeginDisabled(!increaseHist);
    if (ImGui::InputInt("factor", &factor, 2))
    {
        // only allow odd factors above 0
        if (factor % 2 == 0) factor -= 1;
        if (factor < 1) factor = 1;
    }
    ImGui::EndDisabled();

    ImGui::Checkbox("mirror around z-axis", &mirrorAroundZ);
    ImGuiUtils::TextTooltip("mirrors all values to fill the negative z range");
    ImGui::SameLine();
    ImGui::Checkbox("cut out zeros", &cutOutZeros);

    ImGui::SeparatorText("special beam shapes");
    if (ImGui::Checkbox("gaussian", &gaussianElectronBeam))
    {
        densityFile = "";
        cylindricalElectronBeam = false;

        // remove bend if unticked
        if (!gaussianElectronBeam)
            noElectronBeamBend = false;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("cylindrical", &cylindricalElectronBeam))
    {
        densityFile = "";
        gaussianElectronBeam = false;

        // remove bend if unticked
        if (!cylindricalElectronBeam)
            noElectronBeamBend = false;
    }
    ImGui::BeginDisabled(!(gaussianElectronBeam || cylindricalElectronBeam));
    ImGui::SameLine();
    ImGui::Checkbox("no bend", &noElectronBeamBend);
    ImGui::InputDouble("radius [m]", &electronBeamRadius);
    ImGui::InputDouble("density [1/m^3]", &electronBeamDensity, 0, 0, "%.2e");
    ImGui::EndDisabled();

    ImGui::PopItemWidth();
    ImGui::EndGroup();

    return false;
}

std::string LabEnergyParameter::ToString() const
{
    std::string result = fmt::format(
        "# Lab Energy Parameter:\n"
        "#   lab energy in center        : {:.3e} eV\n"
        "#   drift tube voltage          : {:.3e} V\n"
        "#   energy file                 : {}\n",
        centerLabEnergy,
        driftTubeVoltage,
        energyFile
    );
    if (uniformLabEnergies)
    {
        result += fmt::format(
        "#   used uniform lab energies\n"
        );
    }
    if (noSpaceCharge)
    {
        result += fmt::format(
        "#   used no space charge\n"
        );
    }
    if (!interpolateEnergy)
    {
        result += fmt::format(
        "#   no interpolation of energies\n"
        );
    }

    return result;
}

void LabEnergyParameter::FromString(std::string& str)
{
    std::string line;
    std::stringstream ss(str);

    while (std::getline(ss, line))
    {
        std::vector<std::string> tokens = FileUtils::SplitLine(line, ":");

        std::string key = tokens.at(0);
        FileUtils::RemoveSubstrings(key, { "#" });
        FileUtils::TrimSpaces(key);

        if (tokens.size() == 1)
        {
            if (key == "used uniform lab energies")
            {
                uniformLabEnergies = true;
            }
            if (key == "used no space charge")
            {
                noSpaceCharge = true;
            }
            if (key == "no interpolation of energies")
            {
                interpolateEnergy = false;
            }
        }
        if (tokens.size() != 2)
        {
            continue;
        }

        std::string value = tokens.at(1);
        FileUtils::TrimSpaces(value);
        if (key != "energy file")
        {
            // remove units
            FileUtils::RemoveSubstrings(value, { "eV", "V" });
        }

        if (key == "lab energy in center")
        {
            centerLabEnergy = std::stod(value);
        }
        else if (key == "drift tube voltage")
        {
            driftTubeVoltage = std::stod(value);
        }
        else if (key == "energy file")
        {
            energyFile = value;
        }
    }
}

bool LabEnergyParameter::ShowControls()
{
    ImGui::BeginGroup();

    ImGui::Checkbox("interpolate", &interpolateEnergy);
    ImGuiUtils::TextTooltip("interpolate the value of the lab energy inside the 3D histogram");

    ImGui::Checkbox("no space charge", &noSpaceCharge);
    ImGuiUtils::TextTooltip("removes space charge effects by using the minimum value in the xy-slice at distance z");

    ImGui::Checkbox("uniform energies", &uniformLabEnergies);
    ImGuiUtils::TextTooltip("fill lab energy histogram with the center lab energy");
    ImGui::EndGroup();

    return false;
}

std::string IonBeamParameter::ToString() const
{
    std::string result = fmt::format(
        "# Ion Beam Parameter:\n"
        "#   shift in x and y            : {:.4f}, {:.4f} m\n"
        "#   horizontal, vertical angle  : {:.4f}, {:.4f} rad\n"
        "#   sigmas (x,y)                : {:.4f}, {:.4f} m\n",
        shift[0],
        shift[1],
        angles[0],
        angles[1],
        sigma[0],
        sigma[1]
    );
    if (doubleGaussian)
    {
        result += fmt::format(
        "#   amplitude                   : {:.4f}\n"
        "#   sigmas2 (x,y)               : {:.4f}, {:.4f} m\n"
        "#   amplitude2                  : {:.4f}\n",
            amplitude,
            sigma2[0],
            sigma2[1],
            amplitude2
        );
    }
    return result;
}

void IonBeamParameter::FromString(std::string& str)
{
    std::string line;
    std::stringstream ss(str);

    while (std::getline(ss, line))
    {
        std::vector<std::string> tokens = FileUtils::SplitLine(line, ":");

        if (tokens.size() != 2)
        {
            continue;
        }

        std::string key = tokens.at(0);
        FileUtils::RemoveSubstrings(key, { "#" });
        FileUtils::TrimSpaces(key);

        std::string value = tokens.at(1);
        // remove units
        FileUtils::RemoveSubstrings(value, { "m", "rad"});
        FileUtils::TrimSpaces(value);

        if (key == "shift in x and y")
        {
            std::vector<std::string> tokens = FileUtils::SplitLine(value, ",");
            for (int i = 0; i < 2; ++i)
            {
                shift[i] = std::stof(tokens.at(i));
            }
        }
        else if (key == "horizontal, vertical angle")
        {
            std::vector<std::string> tokens = FileUtils::SplitLine(value, ",");
            for (int i = 0; i < 2; ++i)
            {
                angles[i] = std::stof(tokens.at(i));
            }
        }
        else if (key == "amplitude")
        {
            amplitude = std::stod(value);
        }
        else if (key == "sigmas (x,y)")
        {
            std::vector<std::string> tokens = FileUtils::SplitLine(value, ",");
            for (int i = 0; i < 2; ++i)
            {
                sigma[i] = std::stof(tokens.at(i));
            }
        }
        else if (key == "amplitude2")
        {
            amplitude2 = std::stod(value);
            doubleGaussian = true;
        }
        else if (key == "sigmas2 (x,y)")
        {
            std::vector<std::string> tokens = FileUtils::SplitLine(value, ",");
            for (int i = 0; i < 2; ++i)
            {
                sigma2[i] = std::stof(tokens.at(i));
            }
        }
    }
}

bool IonBeamParameter::ShowControls()
{
    bool somethingChanged = false;

    ImGui::BeginGroup();

    ImGui::PushItemWidth(170.0f);

    somethingChanged |= ImGui::InputFloat2("shift in x and y [m]", shift, "%.4f");
    somethingChanged |= ImGui::InputFloat2("horizontal, vertical angles [rad]", angles, "%.4f");

    ImGui::Separator();
    ImGui::BeginDisabled(!doubleGaussian);
    somethingChanged |= ImGui::InputDouble("amplitude", &amplitude, 0.0f, 0.0f, "%.4f");
    ImGui::EndDisabled();
    somethingChanged |= ImGui::InputFloat2("sigmas x and y [m]", sigma, "%.5f");

    if (ImGui::Button("Set Emittance Values"))
    {
        somethingChanged = true;
        sigma[0] = 0.01044;
        sigma[1] = 0.00455;
    }

    ImGui::Separator();
    somethingChanged |= ImGui::Checkbox("use second gaussian", &doubleGaussian);
    ImGui::BeginDisabled(!doubleGaussian);
    somethingChanged |= ImGui::InputDouble("amplitude 2", &amplitude2, 0.0f, 0.0f, "%.4f");
    somethingChanged |= ImGui::InputFloat2("sigmas 2 x and y [m]", sigma2, "%.5f");

    if (ImGui::Button("Set Lucias Values"))
    {
        somethingChanged = true;
        amplitude = 10.1;
        amplitude2 = 8.1;

        sigma[0] = 9.5e-3f;
        sigma[1] = 5.71e-3f;

        sigma2[0] = 1.39e-3f;
        sigma2[1] = 2.15e-3f;
    }

    ImGui::EndDisabled();

    ImGui::PopItemWidth();
    ImGui::EndGroup();

    return somethingChanged;
}

std::string OutputParameter::ToString() const
{
    return fmt::format(
        "# Output Parameter:\n"
        "#   fit range                   : {:.4e}, {:.4e} eV\n"
        "#   fit detuning energy         : {:.6e} eV\n"
        "#   fit longitudinal kT         : {:.2e} eV\n"
        "#   fit transverse kT           : {:.2e} eV\n"
        "#   fit FWHM                    : {:.4f} eV\n"
        "#   fit scaling factor          : {:.3f}\n"
        "#   effective length            : {:.3f} m\n"
        "#   FWHM                        : {:.4f} eV\n"
        "#   main peak position          : {:.3e} eV\n"
        "#   FWHM distances (left/right) : {:.3e}, {:.3e} eV\n",
        fitRange[0],
        fitRange[1],
        fitDetuningEnergy,
        fitLongitudinalTemperature,
        fitTransverseTemperature,
        fitFWHM,
        fitScalingFactor,
        effectiveLength,
        FWHM,
        mainPeakPosition,
        distancesFWHM[0],
        distancesFWHM[1]
    );
}

void OutputParameter::FromString(std::string& str)
{
    std::string line;
    std::stringstream ss(str);

    while (std::getline(ss, line))
    {
        std::vector<std::string> tokens = FileUtils::SplitLine(line, ":");

        if (tokens.size() != 2)
        {
            continue;
        }

        std::string key = tokens.at(0);
        FileUtils::RemoveSubstrings(key, { "#" });
        FileUtils::TrimSpaces(key);

        std::string value = tokens.at(1);
        // remove units
        FileUtils::RemoveSubstrings(value, { "m", "eV" });
        FileUtils::TrimSpaces(value);

        if (key == "fit range")
        {
            std::vector<std::string> tokens = FileUtils::SplitLine(value, ",");
            for (int i = 0; i < 2; ++i)
            {
                fitRange[i] = std::stof(tokens.at(i));
            }
        }
        else if (key == "fit detuning energy")
        {
            fitDetuningEnergy = std::stod(value);
        }
        else if (key == "fit longitudinal kT")
        {
            fitLongitudinalTemperature = std::stod(value);
        }
        else if (key == "fit transverse kT")
        {
            fitTransverseTemperature = std::stod(value);
        }
        else if (key == "fit FWHM")
        {
            fitFWHM = std::stod(value);
        }
        else if (key == "fit scaling factor")
        {
            fitScalingFactor = std::stod(value);
        }
        else if (key == "effective length")
        {
            effectiveLength = std::stod(value);
        }
        else if (key == "FWHM")
        {
            FWHM = std::stod(value);
        }
        else if (key == "main peak position")
        {
            mainPeakPosition = std::stod(value);
        }
        else if (key == "FWHM distances (left/right)")
        {
            std::vector<std::string> tokens = FileUtils::SplitLine(value, ",");
            for (int i = 0; i < 2; ++i)
            {
                distancesFWHM[i] = std::stof(tokens.at(i));
            }
        }
    }
}

bool OutputParameter::ShowControls()
{
    return false;
}

std::string GeneralParameter::ToString() const
{
    std::string result = fmt::format(
        "# General Parameter:\n"
        "#   limited z range             : {:.3f}, {:.3f} m\n",
        limitedZRange[0],
        limitedZRange[1]
    );
    if (limitZRange)
    {
        return result;
    }
    return std::string();
}

bool GeneralParameter::ShowControls()
{
    ImGui::BeginGroup();

    ImGui::SetNextItemWidth(170.0f);
    ImGui::BeginDisabled(!limitZRange);
    ImGui::InputFloat2("##limited z range", limitedZRange);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Checkbox("limit z range", &limitZRange);

    ImGui::EndGroup();

    return false;
}

void GeneralParameter::FromString(std::string& str)
{
    std::string line;
    std::stringstream ss(str);

    while (std::getline(ss, line))
    {
        std::vector<std::string> tokens = FileUtils::SplitLine(line, ":");

        if (tokens.size() != 2)
        {
            continue;
        }

        std::string key = tokens.at(0);
        FileUtils::RemoveSubstrings(key, { "#" });
        FileUtils::TrimSpaces(key);

        std::string value = tokens.at(1);
        // remove units
        FileUtils::RemoveSubstrings(value, { "m" });
        FileUtils::TrimSpaces(value);

        if (key == "limited z range")
        {
            std::vector<std::string> tokens = FileUtils::SplitLine(value, ",");
            for (int i = 0; i < 2; ++i)
            {
                limitedZRange[i] = std::stof(tokens.at(i));
            }
            limitZRange = true;
        }
    }
}

