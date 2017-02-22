// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

/* global after, before, describe, it */
/* eslint no-invalid-this: "off" */
'use strict';
const emitter = require('events').EventEmitter;
const assert = require('assert');
const pt = require('bindings')('realsense_person_tracking');

function inherits(target, source) {
  // eslint-disable-next-line
  for (let k in source.prototype) {
    target.prototype[k] = source.prototype[k];
  }
}
inherits(pt.PersonTracker, emitter);

let obj;
let personTracker;
describe('SkeletonPointInfo API Test', function() {
  describe('API Existance', function() {
    before(function(done) {
      let fDone = false;
      let personTrackerConfig = {};
      personTrackerConfig = {
        tracking: {
          trackingMode: 'interactive',
        },
        skeleton: {
          enable: true,
        },
      };
      pt.createPersonTracker(personTrackerConfig).then((instance) => {
        personTracker = instance;
        personTracker.on('persontracked', function(result) {
          let person = result.persons[0];
          if (person && person.skeletonInfo.skeletonJoints[0].jointType) {
            obj = person.skeletonInfo.skeletonJoints[0];
            if (!fDone) {
              fDone = true;
              done();
            }
          }
        });
        personTracker.start();
      });
      // eslint-disable-next-line
      this.timeout(60000);
    });
    after(function() {
      return personTracker.stop();
    });

    it('SkeletonPointInfo is exist and type is correct', function() {
      assert.equal(typeof (obj), 'object');
    });

    it('SkeletonPointInfo object has an attribute named jointType, type is string', function() {
      assert.equal(typeof (obj.jointType), 'string');
    });

    it('jointType of SkeletonPointInfo is readonly', function() {
      assert.equal(Object.getOwnPropertyDescriptor(obj, 'jointType').writable, false);
    });

    it('SkeletonPointInfo object has an attribute named worldCoordinate, type is object',
      function() {
        assert.equal(typeof (obj.worldCoordinate), 'object');
      }
    );

    it('worldCoordinate of SkeletonPointInfo is readonly', function() {
      assert.equal(Object.getOwnPropertyDescriptor(obj, 'worldCoordinate').writable, false);
    });

    it('SkeletonPointInfo object has an attribute named imageCoordinate, type is object',
      function() {
        assert.equal(typeof (obj.imageCoordinate), 'object');
      }
    );

    it('imageCoordinate of SkeletonPointInfo is readonly', function() {
      assert.equal(Object.getOwnPropertyDescriptor(obj, 'imageCoordinate').writable, false);
    });

    it('SkeletonPointInfo object has an attribute named worldConfidence, type is number',
      function() {
        assert.equal(typeof (obj.worldConfidence), 'number');
      }
    );

    it('worldConfidence of SkeletonPointInfo is readonly', function() {
      assert.equal(Object.getOwnPropertyDescriptor(obj, 'worldConfidence').writable, false);
    });

    it('SkeletonPointInfo object has an attribute named imageConfidence, type is number',
      function() {
        assert.equal(typeof (obj.imageConfidence), 'number');
      }
    );

    it('imageConfidence of SkeletonPointInfo is readonly', function() {
      assert.equal(Object.getOwnPropertyDescriptor(obj, 'imageConfidence').writable, false);
    });
  });
});
