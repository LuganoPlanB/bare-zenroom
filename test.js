const test = require('brittle')
const addon = require('.')

test('exec returns the planned result shape', (t) => {
  const result = addon.exec('Given nothing')

  t.is(typeof result.exitCode, 'number')
  t.is(typeof result.stdout, 'string')
  t.is(typeof result.stderr, 'string')
  t.absent(result.output)
})
