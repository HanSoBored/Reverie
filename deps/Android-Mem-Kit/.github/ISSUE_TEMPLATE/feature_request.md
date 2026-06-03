---
name: Feature Request
description: Suggest a new feature or improvement
title: "[FEATURE] "
labels: ["enhancement"]
body:
  - type: markdown
    attributes:
      value: |
        Thanks for suggesting a feature! Please fill out the details below.
        
  - type: textarea
    id: problem
    attributes:
      label: Problem Statement
      description: Is your feature request related to a problem?
      placeholder: I'm always frustrated when...
      
  - type: textarea
    id: solution
    attributes:
      label: Proposed Solution
      description: Describe what you want to happen
      placeholder: I would like to see...
    validations:
      required: true
      
  - type: textarea
    id: alternatives
    attributes:
      label: Alternatives Considered
      description: Other solutions you've considered
      placeholder: I've also thought about...
      
  - type: textarea
    id: use_case
    attributes:
      label: Use Case
      description: How would this be used? (security research context)
      placeholder: This would help with...
    validations:
      required: true
      
  - type: textarea
    id: code_example
    attributes:
      label: Example Usage
      description: How would the API look?
      render: c
      
  - type: checkboxes
    id: research
    attributes:
      label: Research Purpose
      description: |
        This library is for security research only. Confirm your use case:
      options:
        - label: This feature is for security research or educational purposes
          required: true
