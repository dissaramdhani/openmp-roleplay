"use strict";

const assert = require("node:assert/strict");
const test = require("node:test");

const {
    checkPawnArchitecture,
    isDependencyAllowed,
} = require("../scripts/check-pawn-architecture");

test("layer dependencies point inward and composition can wire adapters", () => {
    assert.equal(isDependencyAllowed("domain", "application"), false);
    assert.equal(isDependencyAllowed("application", "infrastructure"), false);
    assert.equal(isDependencyAllowed("infrastructure", "application"), true);
    assert.equal(isDependencyAllowed("presentation", "application"), true);
    assert.equal(isDependencyAllowed("presentation", "infrastructure"), false);
    assert.equal(isDependencyAllowed("composition", "infrastructure"), true);
});

test("Pawn sources obey the dependency rules and register the starter modules", () => {
    const result = checkPawnArchitecture();

    assert.deepEqual(result.errors, []);
    assert.ok(result.checkedFiles >= 5);
});
