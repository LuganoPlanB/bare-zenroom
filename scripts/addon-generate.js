#!/usr/bin/env node

const { spawnSync } = require('node:child_process')

// Respect explicit CLI flags first, then environment overrides, then defaults.
function hasFlag(args, longFlag, shortFlag) {
  return args.includes(longFlag) || args.includes(shortFlag)
}

function resolvePlatform(argv) {
  if (hasFlag(argv, '--platform', '-p')) return null
  if (process.env.BARE_MAKE_PLATFORM) return process.env.BARE_MAKE_PLATFORM
  if (process.platform === 'win32') return 'mingw'
  return null
}

function resolveArch(argv) {
  if (hasFlag(argv, '--arch', '-a')) return null
  if (process.env.BARE_MAKE_ARCH) return process.env.BARE_MAKE_ARCH
  if (process.platform === 'win32') return process.arch
  return null
}

function shouldDisableColor(argv, platform) {
  if (argv.includes('--no-color')) return false
  return platform === 'mingw'
}

function main() {
  const argv = process.argv.slice(2)
  const platform = resolvePlatform(argv)
  const arch = resolveArch(argv)
  const args = ['bare-make', 'generate', ...argv]

  if (platform) args.push('--platform', platform)
  if (arch) args.push('--arch', arch)
  if (shouldDisableColor(argv, platform)) {
    args.push('--no-color')
  }

  const result = spawnSync(
    process.platform === 'win32' ? 'npx.cmd' : 'npx',
    args,
    {
      stdio: 'inherit'
    }
  )

  if (typeof result.status === 'number') process.exit(result.status)

  console.error('Failed to start bare-make generate')
  process.exit(1)
}

main()
