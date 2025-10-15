# SprintLite — Minimal JIRA-style Project Manager (MERN)

A simple, production-ready project management app for software teams.  
Plan work, manage a product backlog, create sprints, assign tasks, and move work across a Kanban board.

This project is designed to be clear to read, easy to run, and good for learning full-stack development.

---

## Features

- Teams and projects
- Product backlog with priorities, story points, and labels
- Sprints: create, start, close, and set capacity
- Tasks: create, edit, assign, and comment
- Board with Backlog, In Progress, and Completed (drag and drop)
- Reorder backlog by drag and drop
- Burndown chart (basic, optional)
- CSV export (optional)
- Role-based access (owner, admin, member) planned

---

## Tech Stack

**Frontend**
- React (Vite)
- React Router
- TanStack Query
- @hello-pangea/dnd for drag and drop
- Axios

**Backend**
- Node.js, Express
- MongoDB Atlas, Mongoose
- CORS, dotenv

**Deployment**
- Frontend: Vercel
- Backend: Render
- Database: MongoDB Atlas (M0 free tier)

---

## Monorepo Structure

```
sprintlite/
├─ web/                     # React app (Vite)
│  ├─ index.html
│  └─ src/
│     ├─ main.jsx
│     ├─ App.jsx
│     ├─ pages/
│     │  └─ Board.jsx
│     ├─ components/
│     │  ├─ Column.jsx
│     │  ├─ IssueCard.jsx
│     │  └─ Toolbar.jsx
│     └─ api/
│        ├─ client.js
│        └─ issues.js
└─ server/                  # Express API
   ├─ .env                  # not committed; use env vars in prod
   └─ src/
      ├─ index.js
      ├─ app.js
      ├─ config/
      │  └─ db.js
      ├─ models/
      │  └─ Issue.js
      └─ routes/
         └─ issue.routes.js
```

---


## Deployment (Vercel + Render + Atlas)

1) **Database (MongoDB Atlas)**
   - Create an M0 cluster
   - Add a database user
   - Allow your IP (or 0.0.0.0/0 for development)
   - Copy the SRV connection string as `MONGO_URI`

2) **Backend (Render)**
   - Create a new Web Service
   - Root directory: `server`
   - Build command: `npm i`
   - Start command: `npm start`
   - Environment variables:
     - `MONGO_URI=<your Atlas URI>`
     - `NODE_ENV=production` (optional)
   - After deploy, note your API base URL, for example:
     - `https://sprintlite-api.onrender.com/api`

3) **Frontend (Vercel)**
   - Import the repo as a Vercel project
   - Root directory: `web`
   - Framework preset: Vite
   - Environment variables (Build and Runtime):
     - `VITE_API_URL=https://sprintlite-api.onrender.com/api`
   - Deploy and note your Vercel URL

4) **CORS on the server**
   - In `server/src/app.js`, allow your Vercel origin:
   ```js
   app.use(cors({
     origin: ["http://localhost:5173", "https://YOUR-VERCEL-APP.vercel.app"],
   }));
   ```

---

## API

Base URL (local): `http://localhost:4000/api`  
Base URL (prod): `https://<your-render-service>.onrender.com/api`

### Issues
- `GET /issues`  
  List all issues sorted for board display.

- `POST /issues`  
  Create a new issue.  
  Body:
  ```json
  { "title": "Set up project", "status": "backlog", "points": 3, "assignee": "Sujal" }
  ```

- `POST /issues/reorder`  
  Reorder issues across columns.  
  Body:
  ```json
  {
    "backlog": ["id1", "id2"],
    "in-progress": ["id3"],
    "done": ["id4"]
  }
  ```

- `POST /issues/dev/clear`  
  Clear all issues. Do not expose in production.

- `POST /issues/dev/seed`  
  Seed sample issues. Do not expose in production.

---

## Frontend Integration

Set the API base URL in `web/src/api/client.js`:
```js
import axios from "axios";
export const api = axios.create({
  baseURL: import.meta.env.VITE_API_URL
});
```

React Query is used for data fetching and caching.  
Drag and drop is powered by `@hello-pangea/dnd`.

---

## Roadmap

- User accounts and authentication (JWT)
- Teams and roles
- Sprints with burndown chart
- Backlog grooming tools
- Comments, labels, and subtasks
- Basic analytics (lead time, cycle time, throughput)
- Tests and CI

---

## License
**Personal License** — Unauthorized copying, distribution, or commercial use is prohibited. For permissions, contact [sujalm7200@gmail.com](mailto:sujalm7200@gmail.com).
