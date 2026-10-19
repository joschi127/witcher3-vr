// Horse and boat camera states of the Witcher 3 VR state bridge.
// The launcher copies the variant for the detected game version to
// content/scripts/local/witcher3vr/vehicle_camera_bridge.ws.
// Next-Gen 4.04: states are wrapped by their short state name.

@wrapMethod(HorseRiding)
function OnGameCameraPostTick(
  out moveData: SCameraMovementData,
  dt: float
) {
  var result: bool;
  var horse: W3HorseComponent;

  W3VR_RestorePendingStateBridgeFov(parent);

  parent.W3VR_ApplyCameraFollowPolicy(true);

  result = wrappedMethod(moveData, dt);

  horse = (W3HorseComponent)vehicle;
  W3VR_PublishStateBridgeChange(
    parent,
    W3VR_GetHorseState(horse),
    parent.IsInCombat()
  );
  return result;
}

// Sailing and SailingPassive override the ordinary CR4Player camera tick, so
// neither can rely on W3VR_GetFootState() to replace a retained FootIdle edge.
// Publish boat authority directly from both native camera-state owners.
@wrapMethod(Sailing)
function OnGameCameraPostTick(
  out moveData: SCameraMovementData,
  dt: float
) {
  var result: bool;

  W3VR_RestorePendingStateBridgeFov(parent);
  parent.W3VR_ApplyCameraFollowPolicy(true);
  result = wrappedMethod(moveData, dt);
  W3VR_PublishStateBridgeChange(parent, 11, parent.IsInCombat());
  return result;
}

@wrapMethod(SailingPassive)
function OnGameCameraPostTick(
  out moveData: SCameraMovementData,
  dt: float
) {
  var result: bool;

  W3VR_RestorePendingStateBridgeFov(parent);
  parent.W3VR_ApplyCameraFollowPolicy(true);
  result = wrappedMethod(moveData, dt);
  W3VR_PublishStateBridgeChange(parent, 11, parent.IsInCombat());
  return result;
}
