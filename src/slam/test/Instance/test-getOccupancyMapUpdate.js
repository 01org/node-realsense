 // Copyright (c) 2016 Intel Corporation. All rights reserved.
 // Use of this source code is governed by a MIT-style license that can be
 // found in the LICENSE file.

'use strict';

const slamAddon = require('bindings')('realsense_slam');
const EventEmitter = require('events').EventEmitter;
const assert = require('assert');

function inherits(target, source) {
  // eslint-disable-next-line guard-for-in
  for (let k in source.prototype) {
    target.prototype[k] = source.prototype[k];
  }
}
inherits(slamAddon.Instance, EventEmitter);

function checkReturn(obj, name, type, option) {
  let Result = true;
  if (type == 'typeof') {
    if(typeof(obj[name]) !== option) {
      Result = false;
    }
  }
  if (type == 'writable') {
    if (Object.getOwnPropertyDescriptor(obj, name).writable !== option) {
      writableResult = false;
      Result = false;
    }
  }
  return Result;
}
function tests(state, options) {
  describe('Slam Instance Test - getOccupancyMapUpdate()', function() {
    let slamInstance = null;
    afterEach(() => {
      return slamInstance.stop();
    });
    it('Should get correct data getOccupancyMapUpdate() -  ' + state, function() {
      return new Promise((resolve, reject) => {
        slamAddon.createInstance().then((Instance) => {
          slamInstance = Instance;
          return slamInstance.start();
        }).then(() => {
          if (options == null) {
            return slamInstance.getOccupancyMapUpdate();
          }else{
            return slamInstance.getOccupancyMapUpdate(options);
          }
        }).then((Data) => {
          // eslint-disable-next-line guard-for-in
          for (let i in caseResult) {
            assert.ok(checkReturn(Data, i, 'writable', caseResult[i].writable));
            assert.ok(checkReturn(Data, i, 'typeof', caseResult[i].type));
          }
          resolve();
        }).catch((err) => {
          reject(err);
        });
      });
    });
  });
}
let caseResult = {
  isDataPending: {type: 'boolean', writable: false},
  tileCount: {type: 'number', writable: false},
  data: {type: 'object', writable: false},
};
// let argv = {
//   minX: 1,
//   minY: 1,
//   maxX: 3,
//   maxY: 3,
// };
tests('Positive', null);
// tests('Negative', argv)
