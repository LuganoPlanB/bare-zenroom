const test = require('brittle')
const addon = require('.')

test('exec returns the planned result shape', (t) => {
  const result = addon.exec('Given nothing')

  t.alike(result, {
    exitCode: 1,
    stdout: '',
    stderr: 'Zenroom native library is not linked yet'
  })
})
