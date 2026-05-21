#pragma once
#include "SimulationMethods.h"
#include <chrono>
#include <numeric>
#include <vector>

class Entity {
public:
    Vec3 position{};
    Vec3 velocity{};
    virtual ~Entity() = default;
    virtual void Update(float dt) = 0;
};

class Projectile final : public Entity {
public:
    float gravity_scale = 1.0f;

    void Update(float dt) override {
        velocity = ApplyGravity(velocity, gravity_scale, dt);
        position = IntegrateMovement(position, velocity, dt);
    }
};

class Firework final : public Entity {
public:
    float lifetime = 3.0f;
    float age      = 0.0f;

    void Update(float dt) override {
        age += dt;
        if (LifecycleCheck(age, lifetime)) {
            age      = 0.0f;
            position = {};
            velocity = {0.0f, 15.0f, 0.0f};
        }
        position = IntegrateMovement(position, velocity, dt);
    }
};

class DragParticle final : public Entity {
public:
    float gravity_scale      = 1.0f;
    float drag_coefficient   = 0.1f;

    void Update(float dt) override {
        velocity = ApplyGravity(velocity, gravity_scale, dt);
        velocity = ApplyDrag(velocity, drag_coefficient, dt);
        position = IntegrateMovement(position, velocity, dt);
    }
};

class OOPSimulation {
public:
    OOPSimulation() = default;

    ~OOPSimulation() {
        for (const auto * e : m_SimulationEntities) delete e;
    }

    SimulationResults operator()(const SimulationParams &params) {
        Setup(params);
        SimulationRun(params);
        return RegisterSimulationResults(params);
    }

private:
    std::vector<Entity*> m_SimulationEntities;
    std::vector<double>  m_StepTimings;

    void Setup(const SimulationParams &params) {
        for (const auto * e : m_SimulationEntities) delete e;
        m_SimulationEntities.clear();
        m_StepTimings.clear();
        m_StepTimings.reserve(params.simulation_steps);
        m_SimulationEntities.reserve(params.entity_count);

        const size_t per_type        = params.entity_count / 3;
        const size_t projectile_count = per_type;
        const size_t firework_count   = per_type;
        const size_t drag_count       = params.entity_count - 2 * per_type;

        for (size_t i = 0; i < projectile_count; i++) {
            auto* p          = new Projectile();
            p->position      = {static_cast<float>(i), 0.0f, 0.0f};
            p->velocity      = {10.0f, 20.0f + static_cast<float>(i) * 0.01f, 0.0f};
            m_SimulationEntities.push_back(p);
        }

        for (size_t i = 0; i < firework_count; i++) {
            auto* f          = new Firework();
            f->position      = {static_cast<float>(i), 0.0f, 0.0f};
            f->velocity      = {0.0f, 15.0f + static_cast<float>(i) * 0.01f, 0.0f};
            m_SimulationEntities.push_back(f);
        }

        for (size_t i = 0; i < drag_count; i++) {
            auto* d          = new DragParticle();
            d->position      = {static_cast<float>(i), 0.0f, 0.0f};
            d->velocity      = {5.0f, 10.0f + static_cast<float>(i) * 0.01f, 0.0f};
            m_SimulationEntities.push_back(d);
        }
    }

    void SimulationRun(const SimulationParams &params) {
        for (size_t step = 0; step < params.simulation_steps; step++) {
            auto start = std::chrono::high_resolution_clock::now();
            SimulationUpdate(params.delta_time);
            auto end   = std::chrono::high_resolution_clock::now();
            RegisterStepMetrics(std::chrono::duration<double, std::milli>(end - start).count());
        }
    }

    void SimulationUpdate(const float dt) const {
        for (auto* entity : m_SimulationEntities)
            entity->Update(dt);
    }

    void RegisterStepMetrics(const double step_time_ms) {
        m_StepTimings.push_back(step_time_ms);
    }

    SimulationResults RegisterSimulationResults(SimulationParams params) {
        double total = std::accumulate(m_StepTimings.begin(), m_StepTimings.end(), 0.0);
        double avg   = m_StepTimings.empty() ? 0.0 : total / (double)m_StepTimings.size();

        SimulationResults results{
            .sim_name             = "OOP",
            .entity_count         = params.entity_count,
            .simulation_steps     = params.simulation_steps,
            .total_time_ms        = total,
            .average_step_time_ms = avg,
            .frametime_graph      = m_StepTimings
        };

        if (params.export_details) {
            results.final_positions.reserve(m_SimulationEntities.size());
            results.final_velocities.reserve(m_SimulationEntities.size());
            for (auto* entity : m_SimulationEntities) {
                results.final_positions.push_back(entity->position);
                results.final_velocities.push_back(entity->velocity);
            }
        }

        return results;
    }
};
