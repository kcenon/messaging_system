# Architecture Decision Records (ADR)

This directory contains Architecture Decision Records (ADRs) for the messaging_system project. ADRs document significant architectural decisions made during the development of the system.

## What is an ADR?

An Architecture Decision Record is a document that captures an important architectural decision made along with its context and consequences.

## ADR Index

| ADR | Title | Status | Date |
|-----|-------|--------|------|
| [001](001-logging-architecture.md) | Logging Architecture - ILogger Interface vs logger_system | Accepted | 2025-12-10 |

## ADR Status

- **Proposed**: The decision is under discussion
- **Accepted**: The decision has been accepted and is being implemented
- **Deprecated**: The decision is no longer valid
- **Superseded**: The decision has been replaced by another decision

## Creating a New ADR

1. Copy the template below
2. Name the file `NNN-title.md` where NNN is the next number
3. Fill in the sections
4. Submit a PR for review

## ADR Template

```markdown
# ADR-NNN: [Title]

## Status

[Proposed | Accepted | Deprecated | Superseded by ADR-XXX]

## Context

[Describe the context and problem statement]

## Decision

[Describe the decision and the reasoning behind it]

## Consequences

### Positive
[List positive consequences]

### Negative
[List negative consequences]

## Alternatives Considered

[List alternatives that were considered]

## References

[List relevant references]
```

## Related Documentation

- [Architecture Overview](../ARCHITECTURE.md)
- [Project Structure](../PROJECT_STRUCTURE.md)
- [System Architecture](../advanced/SYSTEM_ARCHITECTURE.md)
