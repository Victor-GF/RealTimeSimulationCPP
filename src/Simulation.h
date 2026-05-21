#pragma once
#include "OOPSimulation.h"
#include "ECSSimulation.h"
#include <iostream>
#include <iomanip>

class Simulation final {
public:
    static void Start(SimulationParams& params) {
        OOPSimulation oop;
        const auto oop_results = oop(params);

        ECSSimulation ecs;
        const auto ecs_results = ecs(params);

        if (params.export_details && params.detailed_results != nullptr) {
            ExportDetailedResults(*params.detailed_results, oop_results, ecs_results);
        } else {
            PrintResults(oop_results);
            PrintResults(ecs_results);
            PrintComparison(oop_results, ecs_results);
        }
    }

private:
    static void PrintResults(const SimulationResults& r) {
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "[" << r.sim_name << "]\n";
        std::cout << "  entities      : " << r.entity_count        << "\n";
        std::cout << "  steps         : " << r.simulation_steps     << "\n";
        std::cout << "  total_ms      : " << r.total_time_ms        << "\n";
        std::cout << "  avg_step_ms   : " << r.average_step_time_ms << "\n\n";
    }

    static void PrintComparison(const SimulationResults& oop, const SimulationResults& ecs) {
        double speedup = oop.total_time_ms / ecs.total_time_ms;
        std::cout << std::setprecision(2);
        std::cout << "ECS speedup over OOP: " << speedup << "x\n";
    }

    static void ExportDetailedResults(SimulationResults& out,
                               const SimulationResults& oop,
                               const SimulationResults& ecs) {
        out = ecs;
        out.sim_name         = "ECS_detailed";
        out.final_positions  = ecs.final_positions;
        out.final_velocities = ecs.final_velocities;
    }
};
