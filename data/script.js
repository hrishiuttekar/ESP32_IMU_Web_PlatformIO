/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-mpu-6050-web-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

let scene, camera, renderer, rocket;

function parentWidth(elem) {
  return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
  return elem.parentElement.clientHeight;
}

function init3D(){
  scene = new THREE.Scene();
  scene.background = new THREE.Color(0xffffff);

  camera = new THREE.PerspectiveCamera(75, parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube")), 0.1, 1000);

  renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));

  document.getElementById('3Dcube').appendChild(renderer.domElement);

  rocket = new THREE.Group();

  const hull = new THREE.MeshStandardMaterial({ color: 0xf1f5f9, roughness: 0.45 });
  const accent = new THREE.MeshStandardMaterial({ color: 0xe74c35, roughness: 0.42 });
  const metal = new THREE.MeshStandardMaterial({ color: 0x263548, roughness: 0.55 });
  const glass = new THREE.MeshStandardMaterial({
    color: 0x178dcb,
    emissive: 0x062c48,
    roughness: 0.25
  });

  function addPart(geometry, material, y) {
    const mesh = new THREE.Mesh(geometry, material);
    mesh.position.y = y;
    rocket.add(mesh);
    return mesh;
  }

  addPart(new THREE.CylinderGeometry(0.49, 0.49, 2.42, 40), hull, -0.13);
  addPart(new THREE.ConeGeometry(0.49, 1.04, 40), accent, 1.60);
  addPart(new THREE.CylinderGeometry(0.501, 0.501, 0.19, 40), accent, -0.87);
  addPart(new THREE.CylinderGeometry(0.25, 0.36, 0.32, 32, 1, true), metal, -1.50);

  const rim = addPart(new THREE.CylinderGeometry(0.225, 0.225, 0.12, 32), metal, 0.47);
  rim.rotation.x = Math.PI / 2;
  rim.position.z = 0.46;
  const window = addPart(new THREE.SphereGeometry(0.18, 24, 16), glass, 0.47);
  window.position.z = 0.52;
  window.scale.z = 0.24;

  const finShape = new THREE.Shape();
  finShape.moveTo(0.45, -0.46);
  finShape.lineTo(1.00, -1.32);
  finShape.lineTo(0.97, -1.51);
  finShape.lineTo(0.45, -1.33);
  finShape.closePath();
  const finGeometry = new THREE.ExtrudeGeometry(finShape, {
    depth: 0.09,
    bevelEnabled: false
  });
  finGeometry.translate(0, 0, -0.045);
  for (let i = 0; i < 4; i++) {
    const fin = new THREE.Mesh(finGeometry, accent);
    fin.rotation.y = Math.PI / 4 + i * Math.PI / 2;
    rocket.add(fin);
  }
  scene.add(rocket);

  scene.add(new THREE.HemisphereLight(0xffffff, 0x8d9bb2, 0.85));
  const keyLight = new THREE.DirectionalLight(0xffffff, 0.95);
  keyLight.position.set(3, 5, 6);
  scene.add(keyLight);
  camera.position.set(0.50, 0.45, 5.2);
  camera.lookAt(0, 0.20, 0);
  renderer.render(scene, camera);
}

// Resize the 3D object when the browser window changes size
function onWindowResize(){
  camera.aspect = parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube"));
  //camera.aspect = window.innerWidth /  window.innerHeight;
  camera.updateProjectionMatrix();
  //renderer.setSize(window.innerWidth, window.innerHeight);
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));
  renderer.render(scene, camera);
}

window.addEventListener('resize', onWindowResize, false);

// Create the 3D representation
init3D();

// Create events for the sensor readings
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('gyro_readings', function(e) {
    //console.log("gyro_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("gyroX").innerHTML = obj.gyroX;
    document.getElementById("gyroY").innerHTML = obj.gyroY;
    document.getElementById("gyroZ").innerHTML = obj.gyroZ;

    // Apply incoming gyro rotations to the rocket
    rocket.rotation.x = obj.gyroY;
    rocket.rotation.z = obj.gyroX;
    rocket.rotation.y = obj.gyroZ;
    renderer.render(scene, camera);
  }, false);

  source.addEventListener('temperature_reading', function(e) {
    console.log("temperature_reading", e.data);
    document.getElementById("temp").innerHTML = e.data;
  }, false);

  source.addEventListener('accelerometer_readings', function(e) {
    console.log("accelerometer_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("accX").innerHTML = obj.accX;
    document.getElementById("accY").innerHTML = obj.accY;
    document.getElementById("accZ").innerHTML = obj.accZ;
  }, false);
}

function resetPosition(element){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/"+element.id, true);
  console.log(element.id);
  xhr.send();
}
