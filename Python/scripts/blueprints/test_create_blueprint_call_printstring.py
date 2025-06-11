#!/usr/bin/env python


import sys
import os
import time
import socket
import json
import logging
from typing import Dict, Any, Optional

# Add the parent directory to the path so we can import the server module
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger("TestBlueprintFunctionCall")

def send_command(sock: socket.socket, command: str, params: Dict[str, Any]) -> Optional[Dict[str, Any]]:
    """Send a command to the Unreal MCP server and get the response."""
    try:
        # Create command object
        command_obj = {
            "type": command,
            "params": params
        }
        
        # Convert to JSON and send
        command_json = json.dumps(command_obj)
        logger.info(f"Sending command: {command_json}")
        sock.sendall(command_json.encode('utf-8'))
        
        # Receive response
        chunks = []
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            chunks.append(chunk)
            
            # Try parsing to see if we have a complete response
            try:
                data = b''.join(chunks)
                json.loads(data.decode('utf-8'))
                # If we can parse it, we have the complete response
                break
            except json.JSONDecodeError:
                # Not a complete JSON object yet, continue receiving
                continue
        
        # Parse response
        data = b''.join(chunks)
        response = json.loads(data.decode('utf-8'))
        logger.info(f"Received response: {response}")
        return response
        
    except Exception as e:
        logger.error(f"Error sending command: {e}")
        return None
    
def main():
    try:
        # Create a socket connection to the Unreal MCP server
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(("localhost", 55557))

        bp_params = {
                "name": "BP_PrintString",
                "parent_class": "Actor"
            }

        # Send a command to create a blueprint
        response = send_command(sock, "create_blueprint", bp_params)

        # Print the response
        if response:
            logger.info(f"Blueprint created successfully: {response}")
        else:
            logger.error("Failed to create blueprint.")

        command_params = {
                    "blueprint_name": "BP_PrintString",
                    "function_or_graph_name": "EventGraph",
                    "target_class": "UKismetSystemLibrary",
                    "target_function": "PrintString"
                }    
        response = send_command(sock, "add_function_call_node", command_params)

        if not response or response.get("status") != "success":
            logger.error(f"Failed to add function call node: {response}")
        else:
            logger.info(f"Function call node added successfully: {response}")

        node_id_printstring = response.get("result").get("node_id")

        set_defaultvalue_params = {
            "blueprint_name": "BP_PrintString",
            "function_or_graph_name": "EventGraph",
            "node_id": node_id_printstring,
            "pin_name": "InString",
            "default_value": "Hello, Unreal MCP!"
        }

        response = send_command(sock, "set_node_pin_default_value", set_defaultvalue_params)
        if not response or response.get("status") != "success":
            logger.error(f"Failed to set default value for print string node: {response}")
            return
        logger.info(f"Default value set successfully for print string node: {response}")

        set_defaultvalue_params = {
            "blueprint_name": "BP_PrintString",
            "function_or_graph_name": "EventGraph",
            "node_id": node_id_printstring,
            "pin_name": "bPrintToLog",
            "default_value": "false"
        }

        response = send_command(sock, "set_node_pin_default_value", set_defaultvalue_params)
        if not response or response.get("status") != "success":
            logger.error(f"Failed to set default value for print string node: {response}")
            return
        logger.info(f"Default value set successfully for print string node: {response}")

        # Find the event node for ReceiveBeginPlay
        find_event_params = {
            "blueprint_name": "BP_PrintString",
            "function_or_graph_name": "EventGraph",
            "event_name": "ReceiveBeginPlay"
        }
        response = send_command(sock, "find_event_node_by_name", find_event_params)
        if not response or response.get("status") != "success":
            logger.error(f"Failed to find event node: {response}")
            return

        logger.info(f"Event node found successfully: {response}")

        node_id_eventstart = response.get("result").get("node_id")

        get_pin_params1 = {
            "blueprint_name": "BP_PrintString",
            "function_or_graph_name": "EventGraph",
            "node_id": node_id_eventstart
        }

        response = send_command(sock, "get_node_pins", get_pin_params1)
        if not response or response.get("status") != "success":
            logger.error(f"Failed to get node pins: {response}")
            return

        logger.info(f"Node pins retrieved successfully: {response}")

        get_pin_params2 = {
            "blueprint_name": "BP_PrintString",
            "function_or_graph_name": "EventGraph",
            "node_id": node_id_printstring
        }

        response = send_command(sock, "get_node_pins", get_pin_params2)
        if not response or response.get("status") != "success":
            logger.error(f"Failed to get node pins for print string: {response}")
            return
        logger.info(f"Node pins for print string retrieved successfully: {response}")
        # Connect the event node to the print string node

        connect_params = {
            "blueprint_name": "BP_PrintString",
            "function_or_graph_name": "EventGraph",
            "source_node_id": node_id_eventstart,
            "target_node_id": node_id_printstring,
            "source_pin_name": "then",
            "target_pin_name": "execute"
        }

        response = send_command(sock, "connect_blueprint_nodes", connect_params)
        if not response or response.get("status") != "success":
            logger.error(f"Failed to connect nodes: {response}")
            return

        logger.info(f"Nodes connected successfully: {response}")

        compile_params = {
            "blueprint_name": "BP_PrintString"
        }

        response = send_command(sock, "compile_blueprint", compile_params)

        # Fixed response check to handle nested structure
        if not response or response.get("status") != "success":
            logger.error(f"Failed to compile blueprint: {response}")
            return
            
        logger.info("Blueprint compiled successfully!")


        get_nodes_params = {
            "blueprint_name": "BP_PrintString",
            "function_or_graph_name": "EventGraph"
        }

        response = send_command(sock, "get_all_nodes", get_nodes_params)
        if not response or response.get("status") != "success":
            logger.error(f"Failed to get all nodes: {response}")
            return

        logger.info(f"All nodes retrieved successfully: {response}")

    except Exception as e:
        logger.error(f"Error: {e}")
        sys.exit(1)

    finally:
        # Close the socket connection
        sock.close()

if __name__ == "__main__":
    main()