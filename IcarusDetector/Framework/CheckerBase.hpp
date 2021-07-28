#pragma once

#include <GaiaFramework/GaiaFramework.hpp>
#include <GaiaComponents/GaiaComponents.hpp>
#include <GaiaTags/GaiaTags.hpp>
#include <unordered_set>
#include <string>

namespace Icarus
{
    template <typename CandidateType,
            typename = typename std::enable_if_t<std::is_base_of_v<Gaia::Components::Component, CandidateType>>,
            typename = typename std::enable_if_t<std::is_base_of_v<Gaia::Tags::Holder, CandidateType>>>
    class CheckerBase
    {
    protected:
        virtual bool CheckScenario(CandidateType* candidate) = 0;
        virtual bool CheckPattern(CandidateType* candidate) = 0;

        /// Add component to the candidate if it pass the pattern check.
        virtual void AddPatternComponent(Gaia::Components::Component* candidate) {}

        std::unordered_set<std::string> ScenarioTags;
        std::unordered_set<std::string> PatternTags;

        Gaia::Framework::Clients::ConfigurationClient* Configurator {nullptr};

    public:
        /// Load configuration here.
        virtual void LoadConfiguration() {};

        /// Initialize basic facilities.
        void Initialize(Gaia::Framework::Clients::ConfigurationClient* configurator)
        {
            Configurator = configurator;

            LoadConfiguration();
        }

        /// Check whether the given candidate satisfy the condition or not.
        bool Check(CandidateType* candidate)
        {
            if (!CheckScenario(candidate)) return false;
            dynamic_cast<Gaia::Tags::Holder*>(candidate)->AddTags(ScenarioTags);
            if (!CheckPattern(candidate)) return false;
            dynamic_cast<Gaia::Tags::Holder*>(candidate)->AddTags(PatternTags);
            AddPatternComponent(candidate);

            return true;
        }
    };
}