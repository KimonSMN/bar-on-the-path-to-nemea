Structure of project:

1. An additional flag, -o open_hours, has been added to the ./receptionist program. Despite being named "hours," this flag takes an input in seconds.

   - This flag determines how long the shop remains open for visitors. During this period, visitors are allowed to enter the shop and place their orders.

2. After the specified time (open_hours) has elapsed, the shop closes:

   - No new visitors are allowed to enter or place orders.
   - Visitors already inside the shop will continue their activities (resting) and leave only after they are finished.

3. If the shop remains open (open_hours is not yet over) but no new visitors arrive, the shop will wait until another visitor enters to close properly.
   - However, if the day is slow, and no visitors come, the shop can also be closed early by manually interrupting it with Ctrl + C.

4. Visitors will attempt to occupy a chair. If an empty chair is available, they will take a seat and place their order. If no chair is available, they will wait by sleeping for 2 seconds before trying again. 
This waiting time, including the sleep duration, is included in the final average wait time calculation by the receptionist at the end of the day.


