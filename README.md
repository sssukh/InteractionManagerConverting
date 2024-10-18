원본 애셋 출처 : https://www.unrealengine.com/marketplace/ko/product/ultimate-interaction-manager

# - 구성
3가지 주요 클래스로 이루어져있습니다.
- Interaction Target : 상호작용 당하는 물체의 ActorComponent로 InnerZone(상호작용 버튼), OuterZone(상호작용 여부) 두개의 SphereComponent를 두고 Collision 여부에 따라 위젯을 활성화시킵니다.
- Interactor : 상호작용하는 플레이어의 ActorComponent로 예비 위젯풀을 만들어두고 Interaction Target에 일정거리 이상 가까워지면 해당 위젯을 활성화합니다.
- Interaction Target Widget : 상호작용 위젯으로 상호작용 여부를 보여주는 체크위젯, 상호작용 버튼을 보여주는 위젯으로 이루어져있고 상호작용 유형에 따라 게이지가 보입니다.

상호작용 유형으로는 Tap, Hold, Repeat이 있습니다.

![image](https://github.com/user-attachments/assets/04778bf7-e914-4ab2-ab9d-c7fdac773741)


# - 기존과 다르게 추가한 점
기존에는 상호작용 중에 고개를 돌려 다른 Interaction Target이 활성화되면 진행상황이 완전히 초기화되었다. Hold의 경우 상호작용이 끝나기 전에 버튼을 떼면 진행상황이 줄어들었다.
이 프레임워크를 컨버팅한 목적이 오버쿡드 같은 게임을 만들기 위함이었는데 요리를 진행하다가 진행과정이 줄어들거나 초기화되는게 어울리지 않아서 수정이 필요했다.
상호작용 중에 고개를 돌려도 Interaction의 진행상황이 초기화되는 부분을 수정했고 Hold의 진행상황이 줄어드는 것 역시 수정했다.

![save](https://github.com/user-attachments/assets/cdeb8749-cb13-4c5b-bccb-18aadb9e44cd)


언리얼 엔진의 StateTree를 도입하려고 했으나 기존 코드의 수정이 많이 필요하다생각되어 도중에 중단했다.

종료 방식이 기존에는 Enum을 사용해서 분리하였지만 비효율적이고 하드코딩적인 부분이 많다고 생각하여 간소화해서 전략패턴을 도입.

- 기존방식(case 문으로 일치하면 처리)

![image](https://github.com/user-attachments/assets/424b3b3e-422a-4059-9e60-60bf9e8aa0fd)
![image](https://github.com/user-attachments/assets/cb1926a8-b680-47c5-a09e-aab7421f4bdb)

- 수정한 방식(클래스 내부에 처리 내용 있음)

![image](https://github.com/user-attachments/assets/12ca86e5-bb4d-418f-a03e-e9a6796e4424)
