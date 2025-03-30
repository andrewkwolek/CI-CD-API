# Save this as client/grpc_client.py

import grpc
import data_transfer_pb2
import data_transfer_pb2_grpc


class ItemTransferClient:
    def __init__(self, server_address='localhost:8082'):
        self.channel = grpc.insecure_channel(server_address)
        self.stub = data_transfer_pb2_grpc.ItemTransferStub(self.channel)

    def get_item(self, item_id, name=''):
        """
        Get an item from the server by ID
        """
        request = data_transfer_pb2.Item(id=item_id, name=name)
        try:
            response = self.stub.GetItem(request)
            return {"id": response.id, "name": response.name}
        except grpc.RpcError as e:
            if e.code() == grpc.StatusCode.NOT_FOUND:
                return None
            else:
                raise Exception(f"gRPC error: {e.details()}")

    def set_item(self, item_id, name):
        """
        Set an item on the server
        """
        request = data_transfer_pb2.Item(id=item_id, name=name)
        try:
            response = self.stub.SetItem(request)
            return {"id": response.id, "name": response.name}
        except grpc.RpcError as e:
            raise Exception(f"gRPC error: {e.details()}")

    def close(self):
        """
        Close the gRPC channel
        """
        self.channel.close()
