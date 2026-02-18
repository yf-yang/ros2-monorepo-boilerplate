project {
  license          = "MIT"
  copyright_holder = "ExampleCorp, inc."
  copyright_year   = 2026

  header_ignore = [
    "ros/build/**",
    "ros/install/**",
    "ros/log/**",
    ".pixi/**",
    ".nx/**",
    "pixi.lock",
    "**/__pycache__/**",
    "**/*.xml",
    "**/*.cfg",
    "**/.devcontainer/**",
  ]
}
