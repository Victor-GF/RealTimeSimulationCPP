#include "Test.h"
#include "ECSSimulation.h"
#include "OOPSimulation.h"
#include "SimulationMethods.h"

// ── Physics functions ─────────────────────────────────────────────────────────

void TestApplyGravityBasic() {
    Vec3 result = ApplyGravity({0.0f, 10.0f, 0.0f}, 1.0f, 1.0f);
    Test::ExpectNear(result.x, 0.0f,           0.001f, "gravity must not change x");
    Test::ExpectNear(result.y, 10.0f - 9.81f,  0.001f, "gravity must reduce y by 9.81/s");
    Test::ExpectNear(result.z, 0.0f,           0.001f, "gravity must not change z");
}

void TestApplyGravityScaled() {
    Vec3 result = ApplyGravity({0.0f, 0.0f, 0.0f}, 2.0f, 1.0f);
    Test::ExpectNear(result.y, -9.81f * 2.0f, 0.001f, "gravity must scale with gravity_scale");
}

void TestApplyGravityDt() {
    Vec3 result = ApplyGravity({0.0f, 0.0f, 0.0f}, 1.0f, 0.5f);
    Test::ExpectNear(result.y, -9.81f * 0.5f, 0.001f, "gravity must scale with dt");
}

void TestApplyDragBasic() {
    Vec3 result = ApplyDrag({10.0f, 10.0f, 0.0f}, 0.1f, 1.0f);
    Test::ExpectNear(result.x, 9.0f, 0.001f, "drag must reduce x velocity");
    Test::ExpectNear(result.y, 9.0f, 0.001f, "drag must reduce y velocity");
}

void TestApplyDragZeroCoeff() {
    Vec3 result = ApplyDrag({5.0f, 5.0f, 5.0f}, 0.0f, 1.0f);
    Test::ExpectNear(result.x, 5.0f, 0.001f, "zero drag must not change velocity");
    Test::ExpectNear(result.y, 5.0f, 0.001f, "zero drag must not change velocity");
}

void TestIntegrateMovementBasic() {
    Vec3 result = IntegrateMovement({0.0f, 0.0f, 0.0f}, {1.0f, 2.0f, 3.0f}, 1.0f);
    Test::ExpectNear(result.x, 1.0f, 0.001f, "position x must advance by velocity.x");
    Test::ExpectNear(result.y, 2.0f, 0.001f, "position y must advance by velocity.y");
    Test::ExpectNear(result.z, 3.0f, 0.001f, "position z must advance by velocity.z");
}

void TestIntegrateMovementDt() {
    Vec3 result = IntegrateMovement({0.0f, 0.0f, 0.0f}, {10.0f, 0.0f, 0.0f}, 0.5f);
    Test::ExpectNear(result.x, 5.0f, 0.001f, "position must scale with dt");
}

void TestLifecycleCheckAlive() {
    Test::Expect(!LifecycleCheck(1.0f, 3.0f), "age < lifetime must return false");
}

void TestLifecycleCheckAtLimit() {
    Test::Expect(LifecycleCheck(3.0f, 3.0f), "age == lifetime must return true");
}

void TestLifecycleCheckOvertime() {
    Test::Expect(LifecycleCheck(5.0f, 3.0f), "age > lifetime must return true");
}

// ── OOP entity updates ────────────────────────────────────────────────────────

void TestProjectileUpdate() {
    Projectile p;
    p.velocity     = {0.0f, 0.0f, 0.0f};
    p.position     = {0.0f, 0.0f, 0.0f};
    p.gravity_scale = 1.0f;
    p.Update(1.0f);
    Test::ExpectNear(p.velocity.y, -9.81f, 0.001f, "projectile velocity.y must decrease by gravity");
    Test::ExpectNear(p.position.y, -9.81f, 0.001f, "projectile position.y must reflect integrated velocity");
}

void TestFireworkLifecycleReset() {
    Firework f;
    f.lifetime = 1.0f;
    f.age      = 0.0f;
    f.position = {100.0f, 200.0f, 0.0f};
    f.velocity = {0.0f, 0.0f, 0.0f};
    f.Update(2.0f);
    Test::ExpectNear(f.age,        0.0f, 0.001f, "firework age must reset to 0");
    Test::ExpectNear(f.position.x, 0.0f + 0.0f * 2.0f, 0.001f, "firework x must be 0 after reset");
}

void TestDragParticleUpdate() {
    DragParticle d;
    d.velocity         = {0.0f, 0.0f, 0.0f};
    d.position         = {0.0f, 0.0f, 0.0f};
    d.gravity_scale    = 1.0f;
    d.drag_coefficient = 0.1f;
    d.Update(1.0f);
    Vec3 expected_vel = ApplyDrag(ApplyGravity({0.0f, 0.0f, 0.0f}, 1.0f, 1.0f), 0.1f, 1.0f);
    Test::ExpectNear(d.velocity.y, expected_vel.y, 0.001f, "drag particle must apply gravity then drag");
}

// ── ECS systems ───────────────────────────────────────────────────────────────

void TestGravitySystem() {
    ECSRegistry r;
    r.projectiles.Resize(1);
    r.drag_particles.Resize(1);
    r.fireworks.Resize(1);

    r.projectiles.velocities[0].velocity    = {0.0f, 0.0f, 0.0f};
    r.projectiles.gravities[0].scale        = 1.0f;
    r.drag_particles.velocities[0].velocity = {0.0f, 0.0f, 0.0f};
    r.drag_particles.gravities[0].scale     = 1.0f;
    r.fireworks.velocities[0].velocity      = {0.0f, 5.0f, 0.0f};

    GravitySystem(r, 1.0f);

    Test::ExpectNear(r.projectiles.velocities[0].velocity.y,    -9.81f, 0.001f, "gravity must affect projectiles");
    Test::ExpectNear(r.drag_particles.velocities[0].velocity.y, -9.81f, 0.001f, "gravity must affect drag particles");
    Test::ExpectNear(r.fireworks.velocities[0].velocity.y,       5.0f,  0.001f, "gravity must NOT affect fireworks");
}

void TestDragSystem() {
    ECSRegistry r;
    r.projectiles.Resize(1);
    r.drag_particles.Resize(1);
    r.fireworks.Resize(0);

    r.projectiles.velocities[0].velocity    = {10.0f, 10.0f, 0.0f};
    r.drag_particles.velocities[0].velocity = {10.0f, 10.0f, 0.0f};
    r.drag_particles.drags[0].coefficient   = 0.1f;

    DragSystem(r, 1.0f);

    Test::ExpectNear(r.projectiles.velocities[0].velocity.x,    10.0f, 0.001f, "drag must NOT affect projectiles");
    Test::ExpectNear(r.drag_particles.velocities[0].velocity.x,  9.0f, 0.001f, "drag must reduce drag particle velocity");
}

void TestIntegrateMovementSystem() {
    ECSRegistry r;
    r.projectiles.Resize(1);
    r.fireworks.Resize(0);
    r.drag_particles.Resize(0);

    r.projectiles.transforms[0].position = {0.0f, 0.0f, 0.0f};
    r.projectiles.velocities[0].velocity = {1.0f, 2.0f, 3.0f};

    IntegrateMovementSystem(r, 1.0f);

    Test::ExpectNear(r.projectiles.transforms[0].position.x, 1.0f, 0.001f, "integrate must move x by velocity");
    Test::ExpectNear(r.projectiles.transforms[0].position.y, 2.0f, 0.001f, "integrate must move y by velocity");
    Test::ExpectNear(r.projectiles.transforms[0].position.z, 3.0f, 0.001f, "integrate must move z by velocity");
}

void TestLifecycleResetSystem() {
    ECSRegistry r;
    r.projectiles.Resize(0);
    r.drag_particles.Resize(0);
    r.fireworks.Resize(1);

    r.fireworks.transforms[0].position    = {100.0f, 200.0f, 0.0f};
    r.fireworks.velocities[0].velocity    = {1.0f, 1.0f, 0.0f};
    r.fireworks.lifetimes[0].lifetime     = 1.0f;
    r.fireworks.lifetimes[0].age          = 0.0f;

    LifecycleResetSystem(r, 2.0f);

    Test::ExpectNear(r.fireworks.lifetimes[0].age,          0.0f, 0.001f, "firework age must reset");
    Test::ExpectNear(r.fireworks.transforms[0].position.x,  0.0f, 0.001f, "firework position must reset");
    Test::ExpectNear(r.fireworks.velocities[0].velocity.y, 15.0f, 0.001f, "firework velocity must reset to launch value");
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main() {
    std::cout << "=== Simulation Methods Tests ===\n\n";

    Test::Run("ApplyGravity basic",              TestApplyGravityBasic);
    Test::Run("ApplyGravity scaled",             TestApplyGravityScaled);
    Test::Run("ApplyGravity with dt",            TestApplyGravityDt);
    Test::Run("ApplyDrag basic",                 TestApplyDragBasic);
    Test::Run("ApplyDrag zero coefficient",      TestApplyDragZeroCoeff);
    Test::Run("IntegrateMovement basic",         TestIntegrateMovementBasic);
    Test::Run("IntegrateMovement with dt",       TestIntegrateMovementDt);
    Test::Run("LifecycleCheck alive",            TestLifecycleCheckAlive);
    Test::Run("LifecycleCheck at limit",         TestLifecycleCheckAtLimit);
    Test::Run("LifecycleCheck overtime",         TestLifecycleCheckOvertime);
    Test::Run("Projectile::Update",              TestProjectileUpdate);
    Test::Run("Firework lifecycle reset",        TestFireworkLifecycleReset);
    Test::Run("DragParticle::Update",            TestDragParticleUpdate);
    Test::Run("GravitySystem",                   TestGravitySystem);
    Test::Run("DragSystem",                      TestDragSystem);
    Test::Run("IntegrateMovementSystem",         TestIntegrateMovementSystem);
    Test::Run("LifecycleResetSystem",            TestLifecycleResetSystem);

    Test::Summary();
    return Test::failed > 0 ? 1 : 0;
}
