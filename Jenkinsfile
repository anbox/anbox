pipeline {
  agent any
  stages {
    stage('Build source') {
      steps {
        sh '/usr/bin/build-source.sh'
        stash(name: 'source', includes: '*.gz,*.bz2,*.xz,*.deb,*.dsc,*.changes,*.buildinfo,lintian.txt')
        cleanWs(cleanWhenAborted: true, cleanWhenFailure: true, cleanWhenNotBuilt: true, cleanWhenSuccess: true, cleanWhenUnstable: true, deleteDirs: true)
      }
    }
    stage('Build binary - armhf') {
      steps {
        parallel(
          "Build binary - armhf": {
            node(label: 'arm64') {
              cleanWs(cleanWhenAborted: true, cleanWhenFailure: true, cleanWhenNotBuilt: true, cleanWhenSuccess: true, cleanWhenUnstable: true, deleteDirs: true)
              unstash 'source'
              sh '''export architecture="armhf"
build-binary.sh'''
              stash(includes: '*.gz,*.bz2,*.xz,*.deb,*.dsc,*.changes,*.buildinfo,lintian.txt', name: 'build-armhf')
              cleanWs(cleanWhenAborted: true, cleanWhenFailure: true, cleanWhenNotBuilt: true, cleanWhenSuccess: true, cleanWhenUnstable: true, deleteDirs: true)
            }


          },
          "Build binary - arm64": {
            node(label: 'arm64') {
              cleanWs(cleanWhenAborted: true, cleanWhenFailure: true, cleanWhenNotBuilt: true, cleanWhenSuccess: true, cleanWhenUnstable: true, deleteDirs: true)
              unstash 'source'
              sh '''export architecture="arm64"
    build-binary.sh'''
              stash(includes: '*.gz,*.bz2,*.xz,*.deb,*.dsc,*.changes,*.buildinfo,lintian.txt', name: 'build-arm64')
              cleanWs(cleanWhenAborted: true, cleanWhenFailure: true, cleanWhenNotBuilt: true, cleanWhenSuccess: true, cleanWhenUnstable: true, deleteDirs: true)
            }
          },
          "Build binary - amd64": {
            node(label: 'amd64') {
              cleanWs(cleanWhenAborted: true, cleanWhenFailure: true, cleanWhenNotBuilt: true, cleanWhenSuccess: true, cleanWhenUnstable: true, deleteDirs: true)
              unstash 'source'
              sh '''export architecture="amd64"
    build-binary.sh'''
              stash(includes: '*.gz,*.bz2,*.xz,*.deb,*.dsc,*.changes,*.buildinfo,lintian.txt', name: 'build-amd64')
              cleanWs(cleanWhenAborted: true, cleanWhenFailure: true, cleanWhenNotBuilt: true, cleanWhenSuccess: true, cleanWhenUnstable: true, deleteDirs: true)
            }
          }
        )
      }
    }
    stage('Results') {
      steps {
        cleanWs(cleanWhenAborted: true, cleanWhenFailure: true, cleanWhenNotBuilt: true, cleanWhenSuccess: true, cleanWhenUnstable: true, deleteDirs: true)
        unstash 'build-armhf'
        unstash 'build-arm64'
        unstash 'build-amd64'
        archiveArtifacts(artifacts: '*.gz,*.bz2,*.xz,*.deb,*.dsc,*.changes,*.buildinfo', fingerprint: true, onlyIfSuccessful: true)
        sh '''/usr/bin/build-repo.sh'''
      }
    }
    stage('Cleanup') {
      steps {
        cleanWs(cleanWhenAborted: true, cleanWhenFailure: true, cleanWhenNotBuilt: true, cleanWhenSuccess: true, cleanWhenUnstable: true, deleteDirs: true)
      }
    }
  }
}
