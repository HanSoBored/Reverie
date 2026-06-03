---
name: Bug Report
description: Report a bug or unexpected behavior
title: "[BUG] "
labels: ["bug"]
body:
  - type: markdown
    attributes:
      value: |
        Thanks for taking the time to fill out this bug report!

  - type: textarea
    id: description
    attributes:
      label: Description
      description: Briefly describe the bug
      placeholder: What happened?
    validations:
      required: true

  - type: textarea
    id: reproduce
    attributes:
      label: Steps to Reproduce
      description: Steps to reproduce the behavior
      placeholder: |
        1. Initialize memkit_hook_init()
        2. Call memkit_patch_create()
        3. See error
    validations:
      required: true

  - type: textarea
    id: expected
    attributes:
      label: Expected Behavior
      description: What should happen instead
      placeholder: What did you expect?
    validations:
      required: true

  - type: input
    id: android_version
    attributes:
      label: Android Version
      placeholder: e.g., Android 13
    validations:
      required: true

  - type: input
    id: device
    attributes:
      label: Device
      placeholder: e.g., Pixel 7, Samsung S23
    validations:
      required: true

  - type: input
    id: ndk_version
    attributes:
      label: NDK Version
      placeholder: e.g., r25b
    validations:
      required: true

  - type: textarea
    id: code
    attributes:
      label: Code Sample
      description: Minimal code to reproduce the issue
      render: c

  - type: textarea
    id: logs
    attributes:
      label: Logs
      description: Relevant logcat output
      render: shell

  - type: checkboxes
    id: terms
    attributes:
      label: Legal Notice
      description: |
        By submitting this issue, you confirm:
        - This is for security research or educational purposes
        - You have authorization to test the target app
      options:
        - label: I agree to use this library responsibly and legally
          required: true
