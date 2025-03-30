pipeline {
    agent any
    
    environment {
        DOCKER_HUB_CREDS = credentials('docker-hub-credentials')
        IMAGE_NAME_SERVER = 'andrewkwolek3/rest-api-server'
        IMAGE_NAME_CLIENT = 'andrewkwolek3/rest-api-client'
        IMAGE_TAG = "${env.BUILD_NUMBER}"
    }
    
    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        
        stage('Create Test Scripts') {
            steps {
                // Create directory for test scripts
                sh 'mkdir -p test-scripts'
                
                // Create the test script
                writeFile file: 'test-scripts/run_integration_tests.sh', text: '''#!/bin/bash
# Integration tests for gRPC application

set -e  # Exit on any error

echo "Starting integration tests..."

# Test 1: API Health Check
echo "Test 1: Checking API health..."
if curl -s -f http://localhost:8002/docs > /dev/null; then
  echo "✅ API is up and docs are accessible"
else
  echo "❌ API health check failed"
  exit 1
fi

# Test 2: Create a new item
echo "Test 2: Creating a new item..."
CREATE_RESPONSE=$(curl -s -X POST http://localhost:8002/items/ \
  -H "Content-Type: application/json" \
  -d '{"id": 1, "name": "Test Item"}')

if echo "$CREATE_RESPONSE" | grep -q "Test Item"; then
  echo "✅ Item created successfully"
else
  echo "❌ Item creation failed"
  echo "Response: $CREATE_RESPONSE"
  exit 1
fi

# Test 3: Retrieve the created item
echo "Test 3: Retrieving item..."
GET_RESPONSE=$(curl -s -X GET http://localhost:8002/items/1)

if echo "$GET_RESPONSE" | grep -q "Test Item"; then
  echo "✅ Item retrieved successfully"
else
  echo "❌ Item retrieval failed"
  echo "Response: $GET_RESPONSE"
  exit 1
fi

# Test 4: Update the item
echo "Test 4: Updating item..."
UPDATE_RESPONSE=$(curl -s -X PUT http://localhost:8002/items/1 \
  -H "Content-Type: application/json" \
  -d '{"id": 1, "name": "Updated Item"}')

if echo "$UPDATE_RESPONSE" | grep -q "Updated Item"; then
  echo "✅ Item updated successfully"
else
  echo "❌ Item update failed"
  echo "Response: $UPDATE_RESPONSE"
  exit 1
fi

# Test 5: Verify the update
echo "Test 5: Verifying update..."
VERIFY_RESPONSE=$(curl -s -X GET http://localhost:8002/items/1)

if echo "$VERIFY_RESPONSE" | grep -q "Updated Item"; then
  echo "✅ Update verified successfully"
else
  echo "❌ Update verification failed"
  echo "Response: $VERIFY_RESPONSE"
  exit 1
fi

# Test 6: Test error handling - Get non-existent item
echo "Test 6: Testing error handling..."
ERROR_RESPONSE=$(curl -s -X GET http://localhost:8002/items/999 -w "%{http_code}" -o /dev/null)

if [ "$ERROR_RESPONSE" = "404" ]; then
  echo "✅ Error handling works correctly"
else
  echo "❌ Error handling failed"
  echo "Expected 404, got: $ERROR_RESPONSE"
  exit 1
fi

echo "🎉 All tests passed successfully!"
exit 0
'''
                
                // Make script executable
                sh 'chmod +x test-scripts/run_integration_tests.sh'
            }
        }
        
        stage('Build Images') {
            steps {
                sh 'docker compose build'
            }
        }
        
        stage('Test') {
            steps {
                // Start containers
                sh 'docker compose up -d'
                
                // Add a delay to ensure services are up
                sh 'sleep 15'
                
                // Run comprehensive integration tests
                sh './test-scripts/run_integration_tests.sh'
                
                // Stop containers
                sh 'docker compose down'
            }
            post {
                failure {
                    // Additional debugging in case of failure
                    sh 'docker compose logs'
                }
            }
        }
        
        stage('Push Images') {
            when {
                branch 'main'  // Only push images when on main branch
            }
            steps {
                sh 'echo $DOCKER_HUB_CREDS_PSW | docker login -u $DOCKER_HUB_CREDS_USR --password-stdin'
                
                // Tag images with build number
                sh "docker tag andrewkwolek3/rest-api-server:main ${IMAGE_NAME_SERVER}:${IMAGE_TAG}"
                sh "docker tag andrewkwolek3/rest-api-client:main ${IMAGE_NAME_CLIENT}:${IMAGE_TAG}"
                
                // Push images with tags
                sh "docker push ${IMAGE_NAME_SERVER}:${IMAGE_TAG}"
                sh "docker push ${IMAGE_NAME_CLIENT}:${IMAGE_TAG}"
                
                // Update latest tag
                sh "docker tag ${IMAGE_NAME_SERVER}:${IMAGE_TAG} ${IMAGE_NAME_SERVER}:latest"
                sh "docker tag ${IMAGE_NAME_CLIENT}:${IMAGE_TAG} ${IMAGE_NAME_CLIENT}:latest"
                sh "docker push ${IMAGE_NAME_SERVER}:latest"
                sh "docker push ${IMAGE_NAME_CLIENT}:latest"
                
                sh 'docker logout'
            }
        }
        
        stage('Deploy') {
            when {
                branch 'main'  // Only deploy when on main branch
            }
            steps {
                // For demonstration, we'll update the docker-compose file and restart the services
                // In a real environment, you might use a Kubernetes deployment or other strategies
                sh 'docker compose down || true'
                sh "sed -i 's/andrewkwolek3\\/rest-api-server:main/andrewkwolek3\\/rest-api-server:${IMAGE_TAG}/g' docker-compose.yml"
                sh "sed -i 's/andrewkwolek3\\/rest-api-client:main/andrewkwolek3\\/rest-api-client:${IMAGE_TAG}/g' docker-compose.yml"
                sh 'docker compose up -d'
                
                // Verify deployment
                sh 'sleep 10'
                sh 'curl -f http://localhost:8002/docs || exit 1'
                echo "Deployment successful!"
            }
        }
    }
    
    post {
        always {
            // Cleanup
            sh 'docker-compose down || true'
            sh 'docker system prune -f'
            deleteDir()
        }
        success {
            echo 'Pipeline completed successfully!'
        }
        failure {
            echo 'Pipeline failed!'
        }
    }
}