"""
Blueprint Node Tools for Unreal MCP.

This module provides tools for manipulating Blueprint graph nodes and connections.
"""

import logging
from typing import Dict, List, Any, Optional
from mcp.server.fastmcp import FastMCP, Context

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_blueprint_node_tools(mcp: FastMCP):
    """Register Blueprint node manipulation tools with the MCP server."""
    
    @mcp.tool()
    def add_blueprint_event_node(
        ctx: Context,
        blueprint_name: str,
        event_name: str,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add an event node to a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            event_name: Name of the event. Use 'Receive' prefix for standard events:
                       - 'ReceiveBeginPlay' for Begin Play
                       - 'ReceiveTick' for Tick
                       - etc.
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            # Handle default value within the method body
            if node_position is None:
                node_position = [0, 0]
            
            params = {
                "blueprint_name": blueprint_name,
                "event_name": event_name,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding event node '{event_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_event_node", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Event node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding event node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_blueprint_input_action_node(
        ctx: Context,
        blueprint_name: str,
        action_name: str,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add an input action event node to a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            action_name: Name of the input action to respond to
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            # Handle default value within the method body
            if node_position is None:
                node_position = [0, 0]
            
            params = {
                "blueprint_name": blueprint_name,
                "action_name": action_name,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding input action node for '{action_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_input_action_node", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Input action node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding input action node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
            
    @mcp.tool()
    def connect_blueprint_nodes(
        ctx: Context,
        blueprint_name: str,
        source_node_id: str,
        source_pin_name: str,
        target_node_id: str,
        target_pin_name: str
    ) -> Dict[str, Any]:
        """
        Connect two nodes in a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            source_node_id: ID of the source node
            source_pin_name: Name of the output pin on the source node
            target_node_id: ID of the target node
            target_pin_name: Name of the input pin on the target node

        Returns:
            Response indicating success or failure
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name,
                "source_node_id": source_node_id,
                "source_pin_name": source_pin_name,
                "target_node_id": target_node_id,
                "target_pin_name": target_pin_name
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Connecting nodes in blueprint '{blueprint_name}'")
            response = unreal.send_command("connect_blueprint_nodes", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Node connection response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error connecting nodes: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_blueprint_variable(
        ctx: Context,
        blueprint_name: str,
        variable_name: str,
        variable_type: str,
        is_exposed: bool = False
    ) -> Dict[str, Any]:
        """
        Add a variable to a Blueprint.
        
        Args:
            blueprint_name: Name of the target Blueprint
            variable_name: Name of the variable
            variable_type: Type of the variable (Boolean, Integer, Float, Vector, etc.)
            is_exposed: Whether to expose the variable to the editor
            
        Returns:
            Response indicating success or failure
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name,
                "variable_name": variable_name,
                "variable_type": variable_type,
                "is_exposed": is_exposed
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding variable '{variable_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_variable", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Variable creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding variable: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_blueprint_get_self_component_reference(
        ctx: Context,
        blueprint_name: str,
        component_name: str,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add a node that gets a reference to a component owned by the current Blueprint.
        This creates a node similar to what you get when dragging a component from the Components panel.
        
        Args:
            blueprint_name: Name of the target Blueprint
            component_name: Name of the component to get a reference to
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            # Handle None case explicitly in the function
            if node_position is None:
                node_position = [0, 0]
            
            params = {
                "blueprint_name": blueprint_name,
                "component_name": component_name,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding self component reference node for '{component_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_get_self_component_reference", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Self component reference node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding self component reference node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_blueprint_self_reference(
        ctx: Context,
        blueprint_name: str,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add a 'Get Self' node to a Blueprint's event graph that returns a reference to this actor.
        
        Args:
            blueprint_name: Name of the target Blueprint
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            if node_position is None:
                node_position = [0, 0]
                
            params = {
                "blueprint_name": blueprint_name,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding self reference node to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_self_reference", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Self reference node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding self reference node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def find_blueprint_nodes(
        ctx: Context,
        blueprint_name: str,
        node_type = None,
        event_type = None
    ) -> Dict[str, Any]:
        """
        Find nodes in a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            node_type: Optional type of node to find (Event, Function, Variable, etc.)
            event_type: Optional specific event type to find (BeginPlay, Tick, etc.)
            
        Returns:
            Response containing array of found node IDs and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name,
                "node_type": node_type,
                "event_type": event_type
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Finding nodes in blueprint '{blueprint_name}'")
            response = unreal.send_command("find_blueprint_nodes", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Node find response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error finding nodes: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    logger.info("Blueprint node tools registered successfully")

    @mcp.tool()
    def add_blueprint_function_node(
        ctx: Context,
        blueprint_name: str,
        target: str,
        function_name: str,
        params = None,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add a function call node to a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            target: Target object for the function (component name or self)
            function_name: Name of the function to call
            params: Optional parameters to set on the function node
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            # Handle default values within the method body
            if params is None:
                params = {}
            if node_position is None:
                node_position = [0, 0]
            
            command_params = {
                "blueprint_name": blueprint_name,
                "target": target,
                "function_name": function_name,
                "params": params,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding function node '{function_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_function_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Function node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding function node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
        
    @mcp.tool()
    def add_function_call_node(            
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str,
        target_class: str,
        target_function: str,
        params = None,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add a function call node to a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            target_class: Class of the target object (e.g., 'Self', 'MyComponent')
            target_function: Specific function to call on the target
            params: Optional parameters to set on the function node
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            # Handle default values within the method body
            if params is None:
                params = {}
            if node_position is None:
                node_position = [0, 0]
            
            command_params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name,
                "target_class": target_class,
                "target_function": target_function,
                "params": params,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding function call node '{function_or_graph_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_function_call_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Function call node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding function call node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}


    @mcp.tool()
    def add_math_node(
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str,
        operation: str,
        data_type: str
    ) -> Dict[str, Any]:
        """
        Add a math operation node to a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            operation: Math operation to perform (Add, Subtract, Multiply, Divide)
            data_type: Type of data (Integer, Float, Vector, etc.)
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            command_params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name,
                "operation": operation,
                "data_type": data_type
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding math node '{operation}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_math_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Math node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding math node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
        
    @mcp.tool()
    def add_control_node(
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str,
        control_type: str
    ) -> Dict[str, Any]:
        """
        Add a control flow node to a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            control_type: Type of control node (Branch, Sequence, ForLoop, etc.)
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            command_params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name,
                "control_type": control_type
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding control node '{control_type}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_control_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Control node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding control node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_sequence_node(
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str
    ) -> Dict[str, Any]:
        """
        Add a sequence node to a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            command_params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding sequence node to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_sequence_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Sequence node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding sequence node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
        
    @mcp.tool()
    def add_select_node(
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str
    ) -> Dict[str, Any]:
        """
        Add a select node to a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            command_params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding select node to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_select_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Select node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding select node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_enum_switch_node(
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str,
        enum_path: str
    ) -> Dict[str, Any]:
        """
        Add an enum switch node to a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            enum_path: Path to the enum type (e.g., '/Game/Enums/MyEnum.MyEnum')
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            command_params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name,
                "enum_path": enum_path
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding enum switch node for '{enum_path}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_enum_switch_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Enum switch node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding enum switch node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
        
    @mcp.tool()
    def add_make_struct_node(            
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str,
        struct_path: str
    ) -> Dict[str, Any]:
        """
        Add a make struct node to a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            struct_path: Path to the struct type (e.g., '/Game/Structs/MyStruct.MyStruct')
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            command_params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name,
                "struct_path": struct_path
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding make struct node for '{struct_path}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_make_struct_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Make struct node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding make struct node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
        
        
    @mcp.tool()
    def add_break_struct_node(            
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str,
        struct_path: str
    ) -> Dict[str, Any]:
        """
        Add a break struct node to a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            struct_path: Path to the struct type (e.g., '/Game/Structs/MyStruct.MyStruct')
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            command_params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name,
                "struct_path": struct_path
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding break struct node for '{struct_path}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_break_struct_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Break struct node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding break struct node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
        
    @mcp.tool()
    def find_event_node_by_name(
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str,
        event_name: str
    ) -> Dict[str, Any]:
        """
        Find an event node in a Blueprint's event graph by its name.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            event_name: Name of the event to find (e.g., 'ReceiveBeginPlay', 'ReceiveTick')
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name,
                "event_name": event_name
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Finding event node '{event_name}' in blueprint '{blueprint_name}'")
            response = unreal.send_command("find_event_node_by_name", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Event node find response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error finding event node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
        
    @mcp.tool()
    def get_node_pins(
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str,
        node_id: str
    ) -> Dict[str, Any]:
        """
        Get the pins of a specific node in a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            node_id: ID of the node to get pins for
            
        Returns:
            Response containing the list of pins and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name,
                "node_id": node_id
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Getting pins for node '{node_id}' in blueprint '{blueprint_name}'")
            response = unreal.send_command("get_node_pins", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Node pins response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error getting node pins: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
        
    @mcp.tool()
    def set_node_pin_default_value(
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str,
        node_id: str,
        pin_name: str,
        default_value: Any
    ) -> Dict[str, Any]:
        """
        Set the default value of a specific pin on a node in a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            node_id: ID of the node containing the pin
            pin_name: Name of the pin to set the default value for
            default_value: The value to set as the default
            
        Returns:
            Response indicating success or failure
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name,
                "node_id": node_id,
                "pin_name": pin_name,
                "default_value": default_value
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Setting default value for pin '{pin_name}' on node '{node_id}' in blueprint '{blueprint_name}'")
            response = unreal.send_command("set_node_pin_default_value", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Set pin default value response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error setting pin default value: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
        
    @mcp.tool()
    def get_all_nodes(
        ctx: Context,
        blueprint_name: str,
        function_or_graph_name: str
    ) -> Dict[str, Any]:
        """
        Get all nodes in a Blueprint's event graph or function graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            function_or_graph_name: Name of the function or event graph
            
        Returns:
            Response containing the list of nodes and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name,
                "function_or_graph_name": function_or_graph_name
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Getting all nodes in blueprint '{blueprint_name}'")
            response = unreal.send_command("get_all_nodes", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"All nodes response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error getting all nodes: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    