<?php

require __DIR__ . '/config.php';

$deviceId = $_POST['id_alat'] ?? null;
$value = $_POST['nilai'] ?? null;
$date = date('Y-m-d H:i:s');

if (!$deviceId || !$value)
    die("Invalid parameters");

$stmt = $conn->prepare("INSERT INTO ph_tanah (id_alat, nilai_ph, created_at) VALUES (?, ?, ?)");
$stmt->bind_param("ids", $deviceId, $value, $date);

if (!$stmt->execute())
    die("Error sql: <br>" . mysqli_error($conn));

echo "New record created successfully";
$conn->close();
