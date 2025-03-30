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
                sh './tests/run_tests.sh'
                
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
            sh 'docker compose down || true'
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