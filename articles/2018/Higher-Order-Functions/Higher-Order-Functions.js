'use strict';
const source = [1, 2, 3, 4, 5, 6];

console.log('every (positive):', source.every(e => e > 0));
console.log('every (odd):', source.every(e => e % 2));
console.log('some (odd):', source.some(e => e % 2));
console.log('some (negative):', source.some(e => e < 0));

console.log('map (square):', source.map(e => e * e));
console.log('filter (odd):', source.filter(e => e % 2));
console.log('reduce (sum):', source.reduce((acc, e) => acc + e, 0));
