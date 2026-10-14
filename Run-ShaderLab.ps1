param(
    [switch]$Test, [switch]$BuildOnly, [switch]$Clean, [switch]$Rebuild,
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release',
    [ValidateSet('x64', 'Win32')][string]$Architecture = 'x64'
)
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
$cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmakeCommand) {
    $labCmake = $cmakeCommand.Source
} else {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    if (!(Test-Path -LiteralPath $vswhere)) { throw 'Install Visual Studio 2022 with Desktop development with C++ and CMake tools.' }
    $vsPath = & $vswhere -latest -version '[17.0,18.0)' -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vsPath) { throw 'Visual Studio 2022 C++ tools were not found.' }
    $labCmake = Join-Path $vsPath 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
}
$labBuildDirectory = Join-Path $PSScriptRoot $(if ($Architecture -eq 'x64') { 'build' } else { 'build-win32' })
if ($Clean) {
    if (Test-Path -LiteralPath (Join-Path $labBuildDirectory 'CMakeCache.txt')) {
        & $labCmake --build $labBuildDirectory --config $Configuration --target clean
        if ($LASTEXITCODE -ne 0) { throw 'Clean failed.' }
    }
    exit 0
}
& $labCmake --preset windows -B $labBuildDirectory -A $Architecture
if ($LASTEXITCODE -ne 0) { throw 'Configuration failed. First build requires Git and an internet connection.' }
$labBuildArguments = @('--build', $labBuildDirectory, '--config', $Configuration, '--parallel')
if ($Rebuild) { $labBuildArguments += '--clean-first' }
& $labCmake @labBuildArguments
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
if ($Test) {
    $labCtest = Join-Path (Split-Path $labCmake) 'ctest.exe'
    & $labCtest --test-dir $labBuildDirectory -C $Configuration --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw 'Validation failed.' }
} elseif (!$BuildOnly) {
    & (Join-Path $labBuildDirectory "$Configuration/ShaderLab.exe")
    if ($LASTEXITCODE -ne 0) { throw 'Shader Lab exited with an error.' }
}
