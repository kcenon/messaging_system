#!/usr/bin/env python3
"""
KCENON Messaging System Python Wrapper

A high-level Python interface for the KCENON messaging system,
providing Pythonic access to the C++ messaging infrastructure.
"""

import json
import threading
import time
from typing import Dict, Any, Optional, Callable, List
from datetime import datetime
import logging

try:
    import messaging_cpp
except ImportError:
    raise ImportError("messaging_cpp module not found. Please ensure the C++ bindings are built.")

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class MessagePriority:
    """Message priority constants"""
    LOW = messaging_cpp.MessagePriority.LOW
    NORMAL = messaging_cpp.MessagePriority.NORMAL
    HIGH = messaging_cpp.MessagePriority.HIGH
    CRITICAL = messaging_cpp.MessagePriority.CRITICAL


class MessagingSystemConfig:
    """Configuration builder for the messaging system"""

    def __init__(self):
        self._builder = messaging_cpp.ConfigBuilder()

    def worker_threads(self, count: int) -> 'MessagingSystemConfig':
        """Set the number of worker threads"""
        self._builder.set_worker_threads(count)
        return self

    def queue_size(self, size: int) -> 'MessagingSystemConfig':
        """Set the maximum queue size"""
        self._builder.set_queue_size(size)
        return self

    def priority_queue(self, enabled: bool = True) -> 'MessagingSystemConfig':
        """Enable or disable priority queue"""
        self._builder.enable_priority_queue(enabled)
        return self

    def compression(self, enabled: bool = True) -> 'MessagingSystemConfig':
        """Enable or disable message compression"""
        self._builder.enable_compression(enabled)
        return self

    def environment(self, env: str) -> 'MessagingSystemConfig':
        """Set the environment (development, staging, production)"""
        self._builder.set_environment(env)
        return self

    def system_name(self, name: str) -> 'MessagingSystemConfig':
        """Set the system name"""
        self._builder.set_system_name(name)
        return self

    def external_logger(self, enabled: bool = True) -> 'MessagingSystemConfig':
        """Enable external logger integration"""
        self._builder.enable_external_logger(enabled)
        return self

    def external_monitoring(self, enabled: bool = True) -> 'MessagingSystemConfig':
        """Enable external monitoring integration"""
        self._builder.enable_external_monitoring(enabled)
        return self

    def build(self):
        """Build the configuration"""
        return self._builder.build()


class Message:
    """Python wrapper for messaging system messages"""

    def __init__(self, topic: str, data: Dict[str, Any],
                 sender: str = "", priority: int = MessagePriority.NORMAL):
        self.topic = topic
        self.data = data
        self.sender = sender
        self.priority = priority
        self.timestamp = datetime.now()

    def to_dict(self) -> Dict[str, Any]:
        """Convert message to dictionary format"""
        return {
            'topic': self.topic,
            'data': self.data,
            'sender': self.sender,
            'priority': self.priority,
            'timestamp': self.timestamp.isoformat()
        }

    @classmethod
    def from_dict(cls, msg_dict: Dict[str, Any]) -> 'Message':
        """Create message from dictionary"""
        return cls(
            topic=msg_dict.get('topic', ''),
            data=msg_dict.get('data', {}),
            sender=msg_dict.get('sender', ''),
            priority=msg_dict.get('priority', MessagePriority.NORMAL)
        )


class MessagingSystem:
    """High-level Python interface to the messaging system"""

    def __init__(self, config: Optional[MessagingSystemConfig] = None):
        """Initialize the messaging system"""
        self._lock = threading.Lock()
        self._handlers = {}  # topic -> list of handlers
        self._running = False

        if config is None:
            self._system = messaging_cpp.create_default_system()
        else:
            config_obj = config.build()
            self._system = messaging_cpp.MessagingSystem(config_obj)

        logger.info("Messaging system created")

    def start(self) -> bool:
        """Start the messaging system"""
        with self._lock:
            if self._running:
                logger.warning("Messaging system is already running")
                return True

            success = self._system.initialize()
            if success:
                self._running = True
                logger.info("Messaging system started successfully")
            else:
                logger.error("Failed to start messaging system")

            return success

    def stop(self) -> None:
        """Stop the messaging system"""
        with self._lock:
            if not self._running:
                logger.warning("Messaging system is not running")
                return

            self._system.shutdown()
            self._running = False
            logger.info("Messaging system stopped")

    def is_running(self) -> bool:
        """Check if the system is running"""
        return self._running and self._system.is_running()

    def publish(self, topic: str, data: Dict[str, Any],
                sender: str = "python_client") -> bool:
        """Publish a message to a topic"""
        if not self._running:
            logger.error("Cannot publish: messaging system is not running")
            return False

        try:
            # Convert Python data to appropriate types
            converted_data = self._convert_data_for_cpp(data)
            result = self._system.publish(topic, converted_data, sender)

            if result:
                logger.debug(f"Published message to topic '{topic}'")
            else:
                logger.warning(f"Failed to publish message to topic '{topic}'")

            return result
        except Exception as e:
            logger.error(f"Error publishing message: {e}")
            return False

    def subscribe(self, topic: str, handler: Callable[[Message], None]) -> None:
        """Subscribe to a topic with a message handler"""
        if not self._running:
            logger.error("Cannot subscribe: messaging system is not running")
            return

        with self._lock:
            if topic not in self._handlers:
                self._handlers[topic] = []

            self._handlers[topic].append(handler)

        # Create wrapper that converts C++ message to Python Message object
        def cpp_handler(msg_dict):
            try:
                message = Message.from_dict(msg_dict)
                handler(message)
            except Exception as e:
                logger.error(f"Error in message handler for topic '{topic}': {e}")

        self._system.subscribe(topic, cpp_handler)
        logger.info(f"Subscribed to topic '{topic}'")

    def get_health(self) -> Dict[str, Any]:
        """Get system health information"""
        if not self._running:
            return {
                'running': False,
                'message_bus_healthy': False,
                'all_services_healthy': False,
                'active_services': 0,
                'total_messages_processed': 0
            }

        health = self._system.check_system_health()
        return {
            'running': self._running,
            'message_bus_healthy': health.message_bus_healthy,
            'all_services_healthy': health.all_services_healthy,
            'active_services': health.active_services,
            'total_messages_processed': health.total_messages_processed,
            'last_check': health.last_check
        }

    def get_subscribed_topics(self) -> List[str]:
        """Get list of topics with active subscriptions"""
        with self._lock:
            return list(self._handlers.keys())

    def _convert_data_for_cpp(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """Convert Python data types to C++ compatible types"""
        converted = {}

        for key, value in data.items():
            if isinstance(value, str):
                converted[key] = value
            elif isinstance(value, int):
                converted[key] = int(value)  # Ensure it's a Python int
            elif isinstance(value, float):
                converted[key] = float(value)
            elif isinstance(value, bool):
                converted[key] = bool(value)
            elif isinstance(value, bytes):
                converted[key] = value
            elif isinstance(value, (list, dict)):
                # Convert complex types to JSON strings
                converted[key] = json.dumps(value)
            else:
                # Convert other types to string
                converted[key] = str(value)

        return converted

    def __enter__(self):
        """Context manager entry"""
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.stop()


class AsyncMessagingSystem(MessagingSystem):
    """Asynchronous version of the messaging system"""

    def __init__(self, config: Optional[MessagingSystemConfig] = None):
        super().__init__(config)
        self._background_thread = None
        self._stop_event = threading.Event()

    def start_async(self) -> bool:
        """Start the messaging system in background mode"""
        if not self.start():
            return False

        self._stop_event.clear()
        self._background_thread = threading.Thread(target=self._background_worker)
        self._background_thread.daemon = True
        self._background_thread.start()

        logger.info("Async messaging system started")
        return True

    def stop_async(self) -> None:
        """Stop the async messaging system"""
        self._stop_event.set()

        if self._background_thread and self._background_thread.is_alive():
            self._background_thread.join(timeout=5.0)

        self.stop()
        logger.info("Async messaging system stopped")

    def _background_worker(self):
        """Background worker for maintenance tasks"""
        while not self._stop_event.is_set():
            try:
                # Perform periodic health checks
                health = self.get_health()
                if not health['message_bus_healthy']:
                    logger.warning("Message bus health check failed")

                # Wait before next check
                self._stop_event.wait(30.0)  # Check every 30 seconds

            except Exception as e:
                logger.error(f"Error in background worker: {e}")
                self._stop_event.wait(5.0)  # Wait 5 seconds on error


# Factory functions for convenience
def create_messaging_system(environment: str = "development") -> MessagingSystem:
    """Create a messaging system for a specific environment"""
    config = MessagingSystemConfig().environment(environment)

    if environment == "production":
        config = config.worker_threads(8).queue_size(50000).compression(True)
    elif environment == "staging":
        config = config.worker_threads(4).queue_size(20000).compression(True)
    else:  # development
        config = config.worker_threads(2).queue_size(5000).compression(False)

    return MessagingSystem(config)


def create_async_messaging_system(environment: str = "development") -> AsyncMessagingSystem:
    """Create an async messaging system for a specific environment"""
    config = MessagingSystemConfig().environment(environment)

    if environment == "production":
        config = config.worker_threads(8).queue_size(50000).compression(True)
    elif environment == "staging":
        config = config.worker_threads(4).queue_size(20000).compression(True)
    else:  # development
        config = config.worker_threads(2).queue_size(5000).compression(False)

    return AsyncMessagingSystem(config)


# Example usage demonstration
def demo_usage():
    """Demonstrate basic usage of the messaging system"""
    print("KCENON Messaging System Python Demo")
    print("=" * 40)

    # Create and configure the messaging system
    with create_messaging_system("development") as msg_system:
        # Set up a message handler
        def handle_user_events(message: Message):
            print(f"Received user event: {message.topic}")
            print(f"Data: {message.data}")
            print(f"Sender: {message.sender}")
            print("-" * 30)

        # Subscribe to user events
        msg_system.subscribe("user.*", handle_user_events)

        # Publish some test messages
        msg_system.publish("user.login", {
            "username": "alice",
            "timestamp": time.time(),
            "ip_address": "192.168.1.100"
        })

        msg_system.publish("user.logout", {
            "username": "alice",
            "session_duration": 3600
        })

        # Wait for message processing
        time.sleep(1)

        # Check system health
        health = msg_system.get_health()
        print(f"System Health: {health}")


if __name__ == "__main__":
    demo_usage()