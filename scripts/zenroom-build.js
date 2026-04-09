#!/usr/bin/env node

const { spawnSync } = require('node:child_process')
const path = require('node:path')

const root = path.resolve(__dirname, '..')
const zenroomDir = path.join(root, 'zenroom')

const targetByPlatform = {
  linux: 'linux-lib',
  darwin: 'osx-lib',
  win32: 'win-dll'
}

function resolveTarget(platform) {
  const override = process.env.ZENROOM_BUILD_TARGET

  if (override) return override

  return targetByPlatform[platform] || null
}

function main() {
  const platform = process.platform
  const arch = process.arch
  const target = resolveTarget(platform)

  if (process.argv.includes('--print-target')) {
    if (!target) {
      console.error(`Unsupported platform: ${platform}/${arch}`)
      process.exit(1)
    }

    console.log(target)
    return
  }

  if (!target) {
    console.error(
      `Unsupported platform for Zenroom local build: ${platform}/${arch}`
    )
    console.error(
      'Set ZENROOM_BUILD_TARGET explicitly if you want to force a Make target.'
    )
    process.exit(1)
  }

  console.log(`Building Zenroom target ${target} for ${platform}/${arch}`)

  const result = spawnSync('make', ['-C', zenroomDir, target], {
    stdio: 'inherit',
    env: {
      ...process.env,
      CCACHE: '1',
      RELEASE: '1'
    }
  })

  if (typeof result.status === 'number') {
    process.exit(result.status)
  }

  console.error('Failed to start make for Zenroom build')
  process.exit(1)
}

main()
