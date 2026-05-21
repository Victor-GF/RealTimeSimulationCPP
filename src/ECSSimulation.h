#pragma once
#include "SimulationMethods.h"
#include <chrono>
#include <functional>
#include <numeric>
#include <vector>

// ── Components ────────────────────────────────────────────────────────────────

struct TransformComponent  { Vec3  position{};               };
struct VelocityComponent   { Vec3  velocity{};               };
struct GravityComponent    { float scale       = 1.0f;       };
struct DragComponent       { float coefficient = 0.1f;       };
struct LifetimeComponent   { float lifetime    = 3.0f;
                             float age         = 0.0f;       };

// ── Archetypes (SoA) ─────────────────────────────────────────────────────────

struct ProjectileArchetype {
    std::vector<TransformComponent> transforms;
    std::vector<VelocityComponent>  velocities;
    std::vector<GravityComponent>   gravities;

    void   Resize(const size_t count) { transforms.resize(count); velocities.resize(count); gravities.resize(count); }
    [[nodiscard]] constexpr size_t Count() const        { return transforms.size(); }
};

struct FireworkArchetype {
    std::vector<TransformComponent> transforms;
    std::vector<VelocityComponent>  velocities;
    std::vector<LifetimeComponent>  lifetimes;

    void   Resize(const size_t count) { transforms.resize(count); velocities.resize(count); lifetimes.resize(count); }
    [[nodiscard]] constexpr size_t Count() const        { return transforms.size(); }
};

struct DragParticleArchetype {
    std::vector<TransformComponent> transforms;
    std::vector<VelocityComponent>  velocities;
    std::vector<GravityComponent>   gravities;
    std::vector<DragComponent>      drags;

    void   Resize(const size_t count) { transforms.resize(count); velocities.resize(count); gravities.resize(count); drags.resize(count); }
    [[nodiscard]] constexpr size_t Count() const        { return transforms.size(); }
};

// ── Registry ─────────────────────────────────────────────────────────────────

class ECSRegistry {
public:
    ProjectileArchetype   projectiles;
    FireworkArchetype     fireworks;
    DragParticleArchetype drag_particles;

    template<typename T>
    constexpr T& GetArchetype();

    template<typename... Ts>
    constexpr auto GetArchetypes() {
        return std::tie(GetArchetype<Ts>()...);
    }
};

template<> constexpr ProjectileArchetype&   ECSRegistry::GetArchetype<ProjectileArchetype>()   { return projectiles;   }
template<> constexpr FireworkArchetype&     ECSRegistry::GetArchetype<FireworkArchetype>()     { return fireworks;     }
template<> constexpr DragParticleArchetype& ECSRegistry::GetArchetype<DragParticleArchetype>() { return drag_particles; }

// ── Systems ───────────────────────────────────────────────────────────────────

constexpr void LifecycleResetSystem(ECSRegistry& registry, const float dt) {
    auto [fireworks] = registry.GetArchetypes<FireworkArchetype>();

    COMPILER_IGNORE_VECTOR_DEPS
    for (size_t i = 0; i < fireworks.Count(); i++) {
        fireworks.lifetimes[i].age += dt;
        if (LifecycleCheck(fireworks.lifetimes[i].age, fireworks.lifetimes[i].lifetime)) {
            fireworks.lifetimes[i].age         = 0.0f;
            fireworks.transforms[i].position   = {};
            fireworks.velocities[i].velocity   = {0.0f, 15.0f, 0.0f};
        }
    }
}

constexpr void GravitySystem(ECSRegistry& registry, const float dt) {
    auto [projectiles, drag_particles] = registry.GetArchetypes<ProjectileArchetype, DragParticleArchetype>();

    COMPILER_IGNORE_VECTOR_DEPS
    for (size_t i = 0; i < projectiles.Count(); i++)
        projectiles.velocities[i].velocity = ApplyGravity(projectiles.velocities[i].velocity, projectiles.gravities[i].scale, dt);
    
    COMPILER_IGNORE_VECTOR_DEPS
    for (size_t i = 0; i < drag_particles.Count(); i++)
        drag_particles.velocities[i].velocity = ApplyGravity(drag_particles.velocities[i].velocity, drag_particles.gravities[i].scale, dt);
}

constexpr void DragSystem(ECSRegistry& registry, const float dt) {
    auto [drag_particles] = registry.GetArchetypes<DragParticleArchetype>();

    COMPILER_IGNORE_VECTOR_DEPS
    for (size_t i = 0; i < drag_particles.Count(); i++)
        drag_particles.velocities[i].velocity = ApplyDrag(drag_particles.velocities[i].velocity, drag_particles.drags[i].coefficient, dt);
}

constexpr void IntegrateMovementSystem(ECSRegistry& registry, const float dt) {
    auto [projectiles, fireworks, drag_particles] = registry.GetArchetypes<ProjectileArchetype, FireworkArchetype, DragParticleArchetype>();

    COMPILER_IGNORE_VECTOR_DEPS
    for (size_t i = 0; i < projectiles.Count(); i++)
        projectiles.transforms[i].position = IntegrateMovement(projectiles.transforms[i].position, projectiles.velocities[i].velocity, dt);

    COMPILER_IGNORE_VECTOR_DEPS
    for (size_t i = 0; i < fireworks.Count(); i++)
        fireworks.transforms[i].position = IntegrateMovement(fireworks.transforms[i].position, fireworks.velocities[i].velocity, dt);
    
    COMPILER_IGNORE_VECTOR_DEPS
    for (size_t i = 0; i < drag_particles.Count(); i++)
        drag_particles.transforms[i].position = IntegrateMovement(drag_particles.transforms[i].position, drag_particles.velocities[i].velocity, dt);
}

// ── ECSSimulation ─────────────────────────────────────────────────────────────

using SystemFn = std::function<void(ECSRegistry&, float)>;

class ECSSimulation {
public:
    ECSSimulation()  = default;
    ~ECSSimulation() = default;

    SimulationResults operator()(const SimulationParams &params) {
        Setup(params);
        SimulationRun(params);
        return RegisterSimulationResults(params);
    }

private:
    ECSRegistry         m_Registry;
    std::vector<SystemFn> m_SimulationPipeline;
    std::vector<double> m_StepTimings;

    void Setup(SimulationParams params) {
        m_StepTimings.clear();
        m_StepTimings.reserve(params.simulation_steps);

        const size_t per_type         = params.entity_count / 3;
        const size_t projectile_count = per_type;
        const size_t firework_count   = per_type;
        const size_t drag_count       = params.entity_count - 2 * per_type;

        m_Registry.projectiles.Resize(projectile_count);
        m_Registry.fireworks.Resize(firework_count);
        m_Registry.drag_particles.Resize(drag_count);

        for (size_t i = 0; i < projectile_count; i++) {
            m_Registry.projectiles.transforms[i].position = {
              static_cast<float>(i), 0.0f, 0.0f};
            m_Registry.projectiles.velocities[i].velocity = {10.0f, 20.0f + static_cast<float>(i) * 0.01f, 0.0f};
        }

        for (size_t i = 0; i < firework_count; i++) {
            m_Registry.fireworks.transforms[i].position = {static_cast<float>(i), 0.0f, 0.0f};
            m_Registry.fireworks.velocities[i].velocity = {0.0f, 15.0f + static_cast<float>(i) * 0.01f, 0.0f};
        }

        for (size_t i = 0; i < drag_count; i++) {
            m_Registry.drag_particles.transforms[i].position = {
              static_cast<float>(i), 0.0f, 0.0f};
            m_Registry.drag_particles.velocities[i].velocity = {5.0f, 10.0f + static_cast<float>(i) * 0.01f, 0.0f};
        }

        m_SimulationPipeline = {
            LifecycleResetSystem,
            GravitySystem,
            DragSystem,
            IntegrateMovementSystem
        };
    }

    void SimulationRun(const SimulationParams &params) {
        for (size_t step = 0; step < params.simulation_steps; step++) {
            auto start = std::chrono::high_resolution_clock::now();
            SimulationUpdate(params.delta_time);
            auto end   = std::chrono::high_resolution_clock::now();
            RegisterStepMetrics(std::chrono::duration<double, std::milli>(end - start).count());
        }
    }

    void SimulationUpdate(const float dt) {
        for (auto& system : m_SimulationPipeline)
            system(m_Registry, dt);
    }

    void RegisterStepMetrics(const double step_time_ms) {
        m_StepTimings.push_back(step_time_ms);
    }

    SimulationResults RegisterSimulationResults(SimulationParams params) {
        double total = std::accumulate(m_StepTimings.begin(), m_StepTimings.end(), 0.0);
        double avg   = m_StepTimings.empty() ? 0.0 : total / (double)m_StepTimings.size();

        SimulationResults results{
            .sim_name             = "ECS",
            .entity_count         = params.entity_count,
            .simulation_steps     = params.simulation_steps,
            .total_time_ms        = total,
            .average_step_time_ms = avg,
            .frametime_graph      = m_StepTimings
        };

        if (params.export_details) {
            auto& p = m_Registry.projectiles;
            auto& f = m_Registry.fireworks;
            auto& d = m_Registry.drag_particles;

            size_t total_entities = p.Count() + f.Count() + d.Count();
            results.final_positions.reserve(total_entities);
            results.final_velocities.reserve(total_entities);

            for (size_t i = 0; i < p.Count(); i++) {
                results.final_positions.push_back(p.transforms[i].position);
                results.final_velocities.push_back(p.velocities[i].velocity);
            }
            for (size_t i = 0; i < f.Count(); i++) {
                results.final_positions.push_back(f.transforms[i].position);
                results.final_velocities.push_back(f.velocities[i].velocity);
            }
            for (size_t i = 0; i < d.Count(); i++) {
                results.final_positions.push_back(d.transforms[i].position);
                results.final_velocities.push_back(d.velocities[i].velocity);
            }
        }

        return results;
    }
};
