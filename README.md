# Setup

Clone the repo.
`git clone <repo>`

Build the application.
`docker compose build`

Start the application.
`docker compose up`

Add an element to the map.
`curl -s -X POST http://localhost:8002/items/ \
  -H "Content-Type: application/json" \
  -d '{"id": 1, "name": "Test Item"}'`

Get element from the map.
`curl -s -X GET http://localhost:8002/items/1`
