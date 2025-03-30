# Save this as api.py

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from client import ItemTransferClient
import uvicorn

app = FastAPI(title="Item Transfer REST API",
              description="REST API for gRPC Item Transfer service")

# Create a global client instance
# In a production environment, you might want to use dependency injection
# or a more sophisticated connection pool
client = ItemTransferClient()

# Pydantic model for Item


class Item(BaseModel):
    id: int
    name: str


@app.get("/items/{item_id}", response_model=Item)
async def get_item(item_id: int):
    """
    Get an item by ID
    """
    result = client.get_item(item_id)
    if not result:
        raise HTTPException(status_code=404, detail="Item not found")
    return result


@app.post("/items/", response_model=Item)
async def create_item(item: Item):
    """
    Create or update an item
    """
    result = client.set_item(item.id, item.name)
    return result


@app.put("/items/{item_id}", response_model=Item)
async def update_item(item_id: int, item: Item):
    """
    Update an item's name
    """
    if item_id != item.id:
        raise HTTPException(
            status_code=400, detail="Path ID does not match item ID")
    result = client.set_item(item.id, item.name)
    return result


@app.on_event("shutdown")
def shutdown_event():
    """
    Close the gRPC channel when the app shuts down
    """
    client.close()


if __name__ == "__main__":
    # Run with uvicorn
    uvicorn.run("api:app", host="0.0.0.0", port=8002, reload=True)
