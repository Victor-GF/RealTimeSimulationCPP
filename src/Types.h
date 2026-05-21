#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;

    constexpr Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    constexpr Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    constexpr Vec3 operator*(float s)       const { return {x * s,   y * s,   z * s};   }
    constexpr Vec3& operator+=(const Vec3& o)     { x += o.x; y += o.y; z += o.z; return *this; }
    constexpr bool  operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
};

struct SimulationResults {
    std::string          sim_name;
    size_t               entity_count;
    size_t               simulation_steps;
    double               total_time_ms;
    double               average_step_time_ms;
    std::vector<double>  frametime_graph;
    std::vector<Vec3>    final_positions;
    std::vector<Vec3>    final_velocities;
};


struct SimulationParams {
    size_t            entity_count;
    size_t            simulation_steps;
    float             delta_time;
    bool              export_details = false;
    SimulationResults* detailed_results = nullptr;

    SimulationParams() = default;
 
    SimulationParams(int argc, char* argv[]) {
        entity_count     = 10000;
        simulation_steps = 1000;
        delta_time       = 0.016f;
        export_details   = false;
        detailed_results = nullptr;
 
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
 
            if (arg == "--entities" && i + 1 < argc)
                entity_count = static_cast<size_t>(std::atoi(argv[++i]));
            else if (arg == "--steps" && i + 1 < argc)
                simulation_steps = static_cast<size_t>(std::atoi(argv[++i]));
            else if (arg == "--dt" && i + 1 < argc)
                delta_time = std::atof(argv[++i]);
            else if (arg == "--export-details")
                export_details = true;
        }
    }
};

#define COMPILER_IGNORE_VECTOR_DEPS _Pragma("GCC ivdep")
