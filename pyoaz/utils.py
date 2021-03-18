def get_keras_model_node_names(model):
    input_node_name = None
    value_node_name = None
    policy_node_name = None
    for node in model.output:
        if "value" in node.name:
            value_node_name = node.name.strip(":0")
        if "policy" in node.name:
            policy_node_name = node.name.strip(":0")
    if "input" in model.input.name:
        input_node_name = model.input.name.strip(":0")
    return input_node_name, value_node_name, policy_node_name
