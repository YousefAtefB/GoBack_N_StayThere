package project;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    @display("bgb=757,455");
    types:
    submodules:
        coordinator: Coordinator {
            @display("p=365,256");
        }
        node1: Node {
            @display("p=472,145");
        }
        node0: Node {
            @display("p=236,145");
        }
    connections:
        coordinator.out1 --> node1.coordinator;
        coordinator.out0 --> node0.coordinator;
        node0.out --> node1.in;
        node1.out --> node0.in;
}

