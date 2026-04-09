const binding = require('./binding')

exports.exec = function exec(script, options = {}) {
  if (typeof script !== 'string') {
    throw new TypeError('script must be a string')
  }

  const { conf = '', keys = '', data = '', extra = '', context = '' } = options

  return binding.exec(script, conf, keys, data, extra, context)
}
