"""
Messaging System Python Module

A Python implementation of the messaging system for network communication
with type-safe containers and client-server architecture.
"""

from .container import Container, Value
from .network import MessagingClient

__version__ = "2.0.0"
__all__ = ["Container", "Value", "MessagingClient"]