#include "ECSSimulation.h"
#include "OOPSimulation.h"
#include "SimulationMethods.h"

static const float EPSILON = 1e-3f;

void TestDetailedResultsHaveCorrectSize() {
    SimulationParams params;
    params.entity_count     = 300;
    params.simulation_steps = 10;
    params.delta_time       = 0.016f;
    params.export_details   = true;

    OOPSimulation oop;
    auto oop_r = oop(params);

    ECSSimulation ecs;
    auto ecs_r = ecs(params);

    Test::Expect(oop_r.final_positions.size()  == params.entity_count, "OOP final_positions size must equal entity_count");
    Test::Expect(oop_r.final_velocities.size() == params.entity_count, "OOP final_velocities size must equal entity_count");
    Test::Expect(ecs_r.final_positions.size()  == params.entity_count, "ECS final_positions size must equal entity_count");
    Test::Expect(ecs_r.final_velocities.size() == params.entity_count, "ECS final_velocities size must equal entity_count");
}


void TestDetailedResultsNotPopulatedWithoutFlag() {
    SimulationParams params;
    params.entity_count     = 300;
    params.simulation_steps = 10;
    params.delta_time       = 0.016f;
    params.export_details   = false;

    OOPSimulation oop;
    auto oop_r = oop(params);

    ECSSimulation ecs;
    auto ecs_r = ecs(params);

    Test::Expect(oop_r.final_positions.empty(),  "OOP final_positions must be empty without export_details");
    Test::Expect(ecs_r.final_positions.empty(),  "ECS final_positions must be empty without export_details");
}

void TestOOPAndECSPositionsMatch() {
    SimulationParams params;
    params.entity_count     = 300;
    params.simulation_steps = 100;
    params.delta_time       = 0.016f;
    params.export_details   = true;

    OOPSimulation oop;
    auto oop_r = oop(params);

    ECSSimulation ecs;
    auto ecs_r = ecs(params);

    for (size_t i = 0; i < params.entity_count; i++) {
        Test::ExpectNear(oop_r.final_positions[i].x, ecs_r.final_positions[i].x, EPSILON, "position.x mismatch");
        Test::ExpectNear(oop_r.final_positions[i].y, ecs_r.final_positions[i].y, EPSILON, "position.y mismatch");
        Test::ExpectNear(oop_r.final_positions[i].z, ecs_r.final_positions[i].z, EPSILON, "position.z mismatch");
    }
}

void TestOOPAndECSVelocitiesMatch() {
    SimulationParams params;
    params.entity_count     = 300;
    params.simulation_steps = 100;
    params.delta_time       = 0.016f;
    params.export_details   = true;

    OOPSimulation oop;
    auto oop_r = oop(params);

    ECSSimulation ecs;
    auto ecs_r = ecs(params);

    for (size_t i = 0; i < params.entity_count; i++) {
        Test::ExpectNear(oop_r.final_velocities[i].x, ecs_r.final_velocities[i].x, EPSILON, "velocity.x mismatch");
        Test::ExpectNear(oop_r.final_velocities[i].y, ecs_r.final_velocities[i].y, EPSILON, "velocity.y mismatch");
        Test::ExpectNear(oop_r.final_velocities[i].z, ecs_r.final_velocities[i].z, EPSILON, "velocity.z mismatch");
    }
}

int main() {
    std::cout << "=== Simulation Results Correctness Tests ===\n\n";

    Test::Run("Detailed arrays have correct size",         TestDetailedResultsHaveCorrectSize);
    Test::Run("Arrays empty without export_details flag",  TestDetailedResultsNotPopulatedWithoutFlag);
    Test::Run("OOP and ECS final positions match",         TestOOPAndECSPositionsMatch);
    Test::Run("OOP and ECS final velocities match",        TestOOPAndECSVelocitiesMatch);

    Test::Summary();
    return Test::failed > 0 ? 1 : 0;
}