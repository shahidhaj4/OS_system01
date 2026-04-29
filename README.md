## Task Distribution – Milestone 1

| Student Name | Responsibility | Files | Tasks |Project Description|
|-------------|---------------|-------|------|-----------------------|
| Shahid Hassan Haj | Main & Output | `main.c` | Integrate all modules, print path and total weight, handle edge cases, ensure compilation |הקוד הזה הוא תוכנית ראשית (main) שמריצה את אלגוריתם דייקסטרה למציאת המסלול הקצר ביותר בין שני צמתים בגרף.
בהתחלה התוכנית בודקת שהמשתמש הכניס קובץ קלט אחד. אם לא – היא מדפיסה הודעת שגיאה.
לאחר מכן היא קוראת את הגרף מתוך הקובץ בעזרת הפונקציה readGraph, וגם מקבלת את צומת המקור (src) וצומת היעד (dst).
יש טיפול במקרה קצה: אם המקור והיעד הם אותו צומת, התוכנית פשוט מדפיסה את הצומת ואת המרחק 0.
אחר כך היא מפעילה את אלגוריתם דייקסטרה על הגרף כדי לחשב את המרחקים הקצרים ביותר מכל צומת, ושומרת את התוצאות במערכים dist (מרחקים) ו-prev (מסלול).
בסיום, התוכנית בודקת אם יש מסלול ליעד:
אם אין (המרחק הוא אינסוף), היא מדפיסה שאין מסלול.
אם כן, היא מדפיסה את המסלול הקצר ביותר ואת המרחק הכולל.
לבסוף היא משחררת את הזיכרון של הגרף ומסיימת את הריצה.
| Miral Agha | File Reading | `file_reader.c`, `file_reader.h` | Read input file, parse nodes/edges, validate input, pass data forward |
| Ghada Bader | Graph Structure | `graph.c`, `graph.h` | Build graph, implement create/add/free functions, manage memory |
| Shahd Julani | Dijkstra Algorithm | `dijkstra.c`, `dijkstra.h` | Implement shortest path, return path + weight, handle special cases |
# OS_system01


