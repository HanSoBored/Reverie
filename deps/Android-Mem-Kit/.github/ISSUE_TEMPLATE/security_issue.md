---
name: Security Issue
description: Report a security vulnerability (private)
title: "[SECURITY] "
labels: ["security"]
body:
  - type: markdown
    attributes:
      value: |
        ## ⚠️ Security Vulnerability Report

        **IMPORTANT:** If this is a security issue in a **target app** (not this library),
        please report it to the app developer directly, NOT here.

        For vulnerabilities in Android-Mem-Kit itself:
        - Email: **raihanzxzy@gmail.com**
        - See [SECURITY.md](../SECURITY.md) for full policy

  - type: dropdown
    id: target
    attributes:
      label: Vulnerability Target
      description: Where is the vulnerability?
      options:
        - Android-Mem-Kit library (report here)
        - Target app (report to app developer)
        - Dependency (ShadowHook/XDL - report upstream)
    validations:
      required: true

  - type: textarea
    id: description
    attributes:
      label: Description
      description: Describe the vulnerability
      placeholder: Detailed description...
    validations:
      required: true

  - type: textarea
    id: impact
    attributes:
      label: Impact
      description: What is the potential impact?
      placeholder: This could allow an attacker to...
    validations:
      required: true

  - type: textarea
    id: reproduce
    attributes:
      label: Steps to Reproduce
      description: How can this be reproduced?
      placeholder: |
        1. ...
        2. ...

  - type: textarea
    id: mitigation
    attributes:
      label: Suggested Mitigation
      description: How could this be fixed?
      placeholder: Possible fix...

  - type: checkboxes
    id: disclosure
    attributes:
      label: Responsible Disclosure
      description: |
        By submitting this report:
        - You agree to responsible disclosure
        - You will not exploit the vulnerability publicly before fix
        - You understand we may coordinate with affected parties
      options:
        - label: I agree to responsible disclosure practices
          required: true
